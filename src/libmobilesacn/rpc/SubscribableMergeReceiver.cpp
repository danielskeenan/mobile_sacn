/**
 * @file SubscribableMergeReceiver.cpp
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/SubscribableMergeReceiver.h>
#include <ranges>
#include <libmobilesacn/util.h>
#include <mobilesacn_messages/ReceiveLevelsResp.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::rpc {

SubscribableMergeReceiver::~SubscribableMergeReceiver()
{
    receiver_.Shutdown();
}

SubscribableMergeReceiver::Ptr SubscribableMergeReceiver::getForUniverse(uint16_t universe)
{
    if (universe == 0) {
        spdlog::critical("Tried to get receiver for universe 0!");
    }
    std::lock_guard receiverLock(SubscribableMergeReceiver::receiversMutex_);
    auto& receiver = receivers_[universe];
    if (!receiver) {
        receiver.reset(new SubscribableMergeReceiver);
        receiver->sacnSettings_.universe_id = universe;
        receiver->sacnSettings_.footprint = { .start_address = 1,
                                              .address_count = DMX_ADDRESS_COUNT };
        receiver->sacnSettings_.use_pap = true;
    }
    return receiver;
}

void SubscribableMergeReceiver::HandleMergedData(sacn::MergeReceiver::Handle handle,
                                                 const SacnRecvMergedData& merged_data)
{
    updateSources(merged_data);

    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, DMX_ADDRESS_COUNT> levelBuf{};
    const auto bufOffset = merged_data.slot_range.start_address - 1;
    const auto bufCount = std::min(merged_data.slot_range.address_count, DMX_ADDRESS_COUNT);

    // Levels
    std::memcpy(levelBuf.data() + bufOffset, merged_data.levels, bufCount);
    auto msgLevels = message::LevelBuffer(levelBuf);
    levelBuf.fill(0);

    // Priorities
    std::memcpy(levelBuf.data() + bufOffset, merged_data.priorities, bufCount);
    auto msgPriorities = message::LevelBuffer(levelBuf);

    // Owners
    auto msgOwners = builder.CreateVectorOfStrings(getOwnerCids(merged_data));

    // Wrap the message.
    auto levelsChangedBuilder = message::LevelsChangedBuilder(builder);
    levelsChangedBuilder.add_levels(&msgLevels);
    levelsChangedBuilder.add_priorities(&msgPriorities);
    levelsChangedBuilder.add_owners(msgOwners);
    auto msgLevelsChanged = levelsChangedBuilder.Finish();

    // Send the message.
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder,
        getNowInMilliseconds(),
        message::ReceiveLevelsRespVal::levelsChanged,
        msgLevelsChanged.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendToSenders(builder.GetBufferPointer(), builder.GetSize());
}

void SubscribableMergeReceiver::HandleSourcesLost(sacn::MergeReceiver::Handle handle,
                                                  uint16_t universe,
                                                  const std::vector<SacnLostSource>& lostSources)
{
    for (const auto& source : lostSources) {
        const auto cid = etcpal::Uuid(source.cid);
        flatbuffers::FlatBufferBuilder builder;
        const auto msgCid = builder.CreateString(cid.ToString());
        auto sourceExpiredBuilder = message::SourceExpiredBuilder(builder);
        sourceExpiredBuilder.add_cid(msgCid);
        const auto msgSourceExpired = sourceExpiredBuilder.Finish();
        const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
            builder,
            getNowInMilliseconds(),
            message::ReceiveLevelsRespVal::sourceExpired,
            msgSourceExpired.Union()
        );
        builder.Finish(msgReceiveLevelsResp);
        sendToSenders(builder.GetBufferPointer(), builder.GetSize());
        sources_.erase(cid);
    }
}

/**
 * Comparing Merge sources is not implemented in the library.
 * @internal
 */
bool operator==(const sacn::MergeReceiver::Source& a, const sacn::MergeReceiver::Source& b)
{
    // Ordered by likeliness of this value changing during level transmission.
    return a.per_address_priorities_active == b.per_address_priorities_active &&
            a.universe_priority == b.universe_priority &&
            a.name == b.name &&
            a.addr == b.addr &&
            a.cid == b.cid;
}

void SubscribableMergeReceiver::updateSources(const SacnRecvMergedData& mergedData)
{
    // If we can't lock, don't wait and try again on the next data packet.
    std::unique_lock sourcesLock(sourcesMutex_, std::try_to_lock_t{});
    if (!sourcesLock) {
        return;
    }
    std::unique_lock sendersLock(sendersMutex_, std::defer_lock_t{});
    // Update sources.
    for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix) {
        const auto newSource = receiver_.GetSource(mergedData.active_sources[ix]);
        Q_ASSERT(newSource);
        const auto cid = newSource->cid;
        auto& oldSource = sources_[cid];
        if (oldSource != *newSource) {
            if (sendersLock || (!sendersLock && sendersLock.try_lock())) {
                // Send an update.
                sendSourceUpdated(*newSource, senders_);
            }
            oldSource = *newSource;
        }
    }
}

void SubscribableMergeReceiver::sendSourceUpdated(const sacn::MergeReceiver::Source& source,
                                                  const SendersList& senders)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(source.cid.ToString());
    const auto msgName = builder.CreateString(source.name);
    const auto msgIpAddr = builder.CreateString(source.addr.ip().ToString());
    auto sourceUpdatedBuilder = message::SourceUpdatedBuilder(builder);
    sourceUpdatedBuilder.add_cid(msgCid);
    sourceUpdatedBuilder.add_name(msgName);
    sourceUpdatedBuilder.add_ipAddr(msgIpAddr);
    sourceUpdatedBuilder.add_hasPap(source.per_address_priorities_active);
    sourceUpdatedBuilder.add_priority(source.universe_priority);
    const auto msgSourceUpdated = sourceUpdatedBuilder.Finish();
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder,
        getNowInMilliseconds(),
        message::ReceiveLevelsRespVal::sourceUpdated,
        msgSourceUpdated.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendToSenders(senders, builder.GetBufferPointer(), builder.GetSize());
}

std::vector<std::string> SubscribableMergeReceiver::getOwnerCids(
    const SacnRecvMergedData& mergedData)
{
    // Get source CIDs.
    std::unordered_map<sacn_remote_source_t, std::string> handleCids;
    handleCids.reserve(sources_.size());
    for (const auto& source : sources_ | std::views::values) {
        handleCids.emplace(source.handle, source.cid.ToString());
    }

    // Create a list of CIDs ordered by address.
    std::vector<std::string> ownerCids;
    ownerCids.reserve(mergedData.slot_range.address_count);
    for (std::size_t addr = 0; addr < mergedData.slot_range.address_count; ++addr) {
        const auto ownerHandle = mergedData.owners[addr];
        // This will push an empty string for an address with no owner.
        ownerCids.push_back(handleCids[ownerHandle]);
    }

    return ownerCids;
}

bool SubscribableMergeReceiver::startup(WsBinarySender* sender)
{
    spdlog::debug("Creating sACN Receiver for univ {}", sacnSettings_.universe_id);
    sacnSettings_.ip_supported = sender->getWsUserData()->sacnNetInt.addr().IsV4()
            ? sacn_ip_support_t::kSacnIpV4Only
            : sacn_ip_support_t::kSacnIpV6Only;
    auto mcastInterfaces = sender->getWsUserData()->sacnMcastInterfaces;
    const auto res = receiver_.Startup(sacnSettings_, *this, mcastInterfaces);
    if (!res.IsOk()) {
        spdlog::critical("Error starting sACN Receiver: {}", res.ToString());
        receiver_.Shutdown();
        return false;
    }

    return true;
}

void SubscribableMergeReceiver::shutdown()
{
    spdlog::debug("Destroying sACN Receiver for univ {}", sacnSettings_.universe_id);
    std::lock_guard lock(receiversMutex_);
    receivers_.erase(sacnSettings_.universe_id);
}

void SubscribableMergeReceiver::onSenderAdded(WsBinarySender* sender)
{
    // Send the new sender information about current sources.
    std::scoped_lock sourcesLock(sourcesMutex_);
    for (const auto& source : sources_ | std::views::values) {
        sendSourceUpdated(source, { sender });
    }
}

}
