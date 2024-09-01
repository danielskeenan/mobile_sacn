/**
 * @file ReceiveLevels.cpp
 *
 * @author Dan Keenan
 * @date 8/30/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/ReceiveLevels.h>
#include <ranges>
#include <mobilesacn_messages/ReceiveLevelsResp.h>
#include <mobilesacn_messages/ReceiveLevelsReq.h>
#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::rpc {

long EtcPalMcastNetintIdComparator(EtcPalMcastNetintId a, EtcPalMcastNetintId b)
{
    return a.index - b.index;
}

SourceDetectorWrapper::~SourceDetectorWrapper()
{
    sacn::SourceDetector::Shutdown();
}

SourceDetectorWrapper& SourceDetectorWrapper::get()
{
    static SourceDetectorWrapper instance;
    return instance;
}

void SourceDetectorWrapper::resetNetworkingIfNeeded(std::vector<SacnMcastInterface>& netints)
{
    const auto newNetIntIds = [&netints]() {
        std::vector<EtcPalMcastNetintId> result;
        result.reserve(netints.size());
        for (const auto& netint : netints) {
            result.push_back(netint.iface);
        }
        std::ranges::sort(result, &EtcPalMcastNetintIdComparator);
        return result;
    }();
    const auto oldNetIntIds = []() {
        auto result = sacn::SourceDetector::GetNetworkInterfaces();
        std::ranges::sort(result, &EtcPalMcastNetintIdComparator);
        return result;
    }();
    if (newNetIntIds != oldNetIntIds) {
        spdlog::debug("Resetting SourceDetector networking as netints have changed.");
        sacn::SourceDetector::ResetNetworking(netints);
    }
}

void SourceDetectorWrapper::addSender(WsBinarySender* sender)
{
    std::scoped_lock sendersLock(sendersMutex_);
    const auto wsUserData = sender->getWsUserData();
    if (senders_.empty()) {
        spdlog::debug("Starting SourceDetector");
        auto settings = sacn::SourceDetector::Settings();
        // Creating the receiver will fail with the default settings if the chosen network interface
        // doesn't have an IPv4 AND an IPv6 address.
        settings.ip_supported = wsUserData->sacnNetInt.addr().IsV4()
                ? sacn_ip_support_t::kSacnIpV4Only
                : sacn_ip_support_t::kSacnIpV6Only;
        auto res = sacn::SourceDetector::Startup(
            settings,
            *this,
            wsUserData->sacnMcastInterfaces
        );
        if (res != kEtcPalErrOk) {
            spdlog::error("Failed to start SourceDetector: {}", res.ToString());
            return;
        }
    } else {
        resetNetworkingIfNeeded(wsUserData->sacnMcastInterfaces);
    }
    senders_.emplace_back(sender);

    // Send the new sender information about current sources.
    std::scoped_lock sourcesLock(sourcesMutex_);
    for (const auto& source : sources_ | std::views::values) {
        sendSourceUpdated(source, { sender });
    }
}

void SourceDetectorWrapper::removeSender(WsBinarySender* sender)
{
    std::scoped_lock sendersLock(sendersMutex_);
    std::erase(senders_, sender);
    if (senders_.empty()) {
        std::scoped_lock sourcesLock(sourcesMutex_);
        spdlog::debug("Stopping SourceDetector");
        sacn::SourceDetector::Shutdown();
        sources_.clear();
    }
}

void SourceDetectorWrapper::HandleSourceUpdated(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name,
    const std::vector<uint16_t>& sourcedUniverses)
{
    std::scoped_lock lock(sendersMutex_, sourcesMutex_);
    auto source = sources_[cid];
    source.cid = cid.ToString();
    source.name = name;
    source.universes = sourcedUniverses;
    spdlog::debug("Source {} ({}) updated with univs {}", source.cid, source.name,
                  fmt::join(sourcedUniverses, ", "));
    sendSourceUpdated(source, senders_);
}

void SourceDetectorWrapper::HandleSourceExpired(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name)
{
    std::scoped_lock lock(sendersMutex_, sourcesMutex_);
    sources_.erase(cid);
    spdlog::debug("Source {} ({}) expired", cid.ToString(), name);
    sendSourceExpired(cid.ToString(), senders_);
}

void SourceDetectorWrapper::sendSourceUpdated(
    const Source& source, const decltype(senders_)& senders)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(source.cid);
    const auto msgName = builder.CreateString(source.name);
    const auto msgUniverses = builder.CreateVector(source.universes);
    auto sourceUpdatedBuilder = message::SourceUpdatedBuilder(builder);
    sourceUpdatedBuilder.add_cid(msgCid);
    sourceUpdatedBuilder.add_name(msgName);
    sourceUpdatedBuilder.add_universes(msgUniverses);
    const auto msgSourceUpdated = sourceUpdatedBuilder.Finish();
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder, message::ReceiveLevelsRespVal::sourceUpdated, msgSourceUpdated.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendToSenders(senders, builder.GetBufferPointer(), builder.GetSize());
}

void SourceDetectorWrapper::sendSourceExpired(
    const std::string& cid, const decltype(senders_)& senders)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(cid);
    auto sourceExpiredBuilder = message::SourceExpiredBuilder(builder);
    sourceExpiredBuilder.add_cid(msgCid);
    const auto msgSourceExpired = sourceExpiredBuilder.Finish();
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder, message::ReceiveLevelsRespVal::sourceExpired, msgSourceExpired.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendToSenders(senders, builder.GetBufferPointer(), builder.GetSize());
}

void SourceDetectorWrapper::sendToSenders(
    const decltype(senders_)& senders, const uint8_t* data, const std::size_t size)
{
    for (auto sender : senders) {
        sender->sendBinary(data, size);
    }
}

ReceiveLevels::~ReceiveLevels()
{
    SourceDetectorWrapper::get().removeSender(this);
    receiver_.Shutdown();
}

void ReceiveLevels::handleConnected()
{
    SourceDetectorWrapper::get().addSender(this);
    sacnSettings_.footprint = { .start_address = 1, .address_count = DMX_ADDRESS_COUNT };
    sacnSettings_.use_pap = true;
    // Creating the receiver will fail with the default settings if the chosen network interface
    // doesn't have an IPv4 AND an IPv6 address.
    sacnSettings_.ip_supported = getWsUserData()->sacnNetInt.addr().IsV4()
            ? sacn_ip_support_t::kSacnIpV4Only
            : sacn_ip_support_t::kSacnIpV6Only;
}

void ReceiveLevels::handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data)
{
    auto msg = message::GetReceiveLevelsReq(data.data());
    if (msg->val_type() == message::ReceiveLevelsReqVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
    }
}

void ReceiveLevels::handleClose()
{
    SourceDetectorWrapper::get().removeSender(this);
    receiver_.Shutdown();
}

void ReceiveLevels::HandleMergedData(sacn::MergeReceiver::Handle handle,
                                     const SacnRecvMergedData& merged_data)
{
    updateSources(merged_data);

    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, DMX_ADDRESS_COUNT> levelBuf{};
    const auto bufOffset = merged_data.slot_range.start_address - 1;
    const auto bufCount = merged_data.slot_range.address_count;

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
        builder, message::ReceiveLevelsRespVal::levelsChanged, msgLevelsChanged.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onChangeUniverse(uint16_t universe)
{
    receiver_.Shutdown();
    sacnSettings_.universe_id = universe;
    if (universe > 0) {
        const auto res = receiver_.Startup(sacnSettings_, *this, getSacnMcastInterfaces());
        if (!res.IsOk()) {
            spdlog::critical("Error starting sACN Receiver: {}", res.ToString());
            receiver_.Shutdown();
            return;
        }
    }
}

/**
 * Comparing Merge sources is not implemented in the library.
 * @internal
 */
bool operator==(const sacn::MergeReceiver::Source& a, const sacn::MergeReceiver::Source& b)
{
    return a.cid == b.cid &&
            a.name == b.name &&
            a.addr == b.addr &&
            a.per_address_priorities_active == b.per_address_priorities_active &&
            a.universe_priority == b.universe_priority;
}

void ReceiveLevels::updateSources(const SacnRecvMergedData& mergedData)
{
    // Track seen CIDs so we know which ones have expired.
    std::set<etcpal::Uuid> oldCids;
    for (const auto& cid : sources_ | std::views::keys) {
        if (cid.IsNull()) {
            continue;
        }
        oldCids.insert(cid);
    }

    // Update sources.
    for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix) {
        const auto newSource = receiver_.GetSource(mergedData.active_sources[ix]);
        Q_ASSERT(newSource);
        const auto cid = newSource->cid;
        auto& oldSource = sources_[cid];
        if (oldSource != *newSource) {
            // Send an update.
            flatbuffers::FlatBufferBuilder builder;
            const auto msgCid = builder.CreateString(cid.ToString());
            const auto msgName = builder.CreateString(newSource->name);
            auto sourceUpdatedBuilder = message::SourceUpdatedBuilder(builder);
            sourceUpdatedBuilder.add_cid(msgCid);
            sourceUpdatedBuilder.add_name(msgName);
            const auto msgSourceUpdated = sourceUpdatedBuilder.Finish();
            const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
                builder, message::ReceiveLevelsRespVal::sourceUpdated, msgSourceUpdated.Union());
            builder.Finish(msgReceiveLevelsResp);
            sendBinary(builder.GetBufferPointer(), builder.GetSize());
            oldSource = *newSource;
        }
        oldCids.erase(cid);
    }

    // Remove expired sources.
    for (const auto& cid : oldCids) {
        flatbuffers::FlatBufferBuilder builder;
        const auto msgCid = builder.CreateString(cid.ToString());
        auto sourceExpiredBuilder = message::SourceExpiredBuilder(builder);
        sourceExpiredBuilder.add_cid(msgCid);
        const auto msgSourceExpired = sourceExpiredBuilder.Finish();
        const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
            builder, message::ReceiveLevelsRespVal::sourceExpired, msgSourceExpired.Union());
        builder.Finish(msgReceiveLevelsResp);
        sendBinary(builder.GetBufferPointer(), builder.GetSize());
        sources_.erase(cid);
    }
}

std::vector<std::string> ReceiveLevels::getOwnerCids(const SacnRecvMergedData& mergedData)
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
        ownerCids.push_back(handleCids[ownerHandle]);
    }

    return ownerCids;
}

} // mobilesacn::rpc
