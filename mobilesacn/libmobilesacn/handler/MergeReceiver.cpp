/**
 * @file MergeReceiver.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "MergeReceiver.h"
#include "mobilesacn/libmobilesacn/SacnSettings.h"
#include <memory>
#include <ranges>
#include <spdlog/spdlog.h>
#include <QSharedPointer>

namespace mobilesacn::handler {

MergeReceiver::~MergeReceiver()
{
    SPDLOG_DEBUG("Deleting sACN Receiver for univ {}", sacnSettings_.universe_id);
    receiver_.Shutdown();
}

MergeReceiver::Ptr MergeReceiver::getForUniverse(uint16_t universe)
{
    if (universe == 0) {
        throw std::logic_error("Cannot get receiver for universe 0");
    }

    std::lock_guard receiverLock(receiversMutex_);
    auto &weakReceiver = receivers_[universe];
    auto receiver = weakReceiver.lock();
    if (!receiver) {
        receiver = std::make_shared<MergeReceiver>();
        weakReceiver = receiver;
        receiver->sacnSettings_.universe_id = universe;
        receiver->sacnSettings_.footprint = {.start_address = 1, .address_count = DMX_ADDRESS_COUNT};
        receiver->sacnSettings_.use_pap = true;
        receiver->startup();
    }
    return receiver;
}

void MergeReceiver::startup()
{
    SPDLOG_DEBUG("Creating sACN Receiver for univ {}", sacnSettings_.universe_id);
    const auto &sacnSettings = SacnSettings::get();
    sacnSettings_.ip_supported = sacnSettings->sacnNetInt.addr().IsV4()
                                     ? sacn_ip_support_t::kSacnIpV4Only
                                     : sacn_ip_support_t::kSacnIpV6Only;
    auto mcastInterfaces = sacnSettings->sacnMcastInterfaces;
    const auto res = receiver_.Startup(sacnSettings_, *this, mcastInterfaces);
    if (!res.IsOk()) {
        SPDLOG_CRITICAL("Error starting sACN Receiver: {}", res.ToString());
        receiver_.Shutdown();
        return;
    }
}

void MergeReceiver::shutdown()
{
    SPDLOG_DEBUG("Destroying sACN Receiver for univ {}", sacnSettings_.universe_id);
    std::lock_guard lock(receiversMutex_);
    receivers_.erase(sacnSettings_.universe_id);
}

std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> MergeReceiver::sources() const
{
    std::scoped_lock sourcesLock(sourcesMutex_);
    // Return a copy to avoid threading issues.
    return {sources_};
}

void MergeReceiver::HandleMergedData(
    sacn::MergeReceiver::Handle handle, const SacnRecvMergedData &merged_data)
{
    updateSources(merged_data);
    const auto ownerCids = getOwnerCids(merged_data);
    Q_EMIT(dataChanged(merged_data, ownerCids));
}

void MergeReceiver::HandleSourcesLost(
    sacn::MergeReceiver::Handle handle,
    uint16_t universe,
    const std::vector<SacnLostSource> &lostSources)
{
    for (const auto &source : lostSources) {
        const auto cid = etcpal::Uuid(source.cid);
        Q_EMIT(sourceLost(cid.ToString()));
        sources_.erase(cid);
    }
}

/**
 * Comparing Merge sources is not implemented in the library.
 * @internal
 */
bool operator==(const sacn::MergeReceiver::Source &a, const sacn::MergeReceiver::Source &b)
{
    // Ordered by likeliness of this value changing during level transmission.
    return a.per_address_priorities_active == b.per_address_priorities_active
           && a.universe_priority == b.universe_priority && a.name == b.name && a.addr == b.addr
           && a.cid == b.cid;
}

void MergeReceiver::updateSources(const SacnRecvMergedData &mergedData)
{
    // If we can't lock, don't wait, just try again on the next data packet.
    std::unique_lock sourcesLock(sourcesMutex_, std::try_to_lock_t{});
    if (!sourcesLock) {
        return;
    }

    // Update sources.
    for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix) {
        const auto newSource = receiver_.GetSource(mergedData.active_sources[ix]);
        Q_ASSERT(newSource);
        const auto cid = newSource->cid;
        auto &oldSource = sources_[cid];
        if (oldSource != *newSource) {
            Q_EMIT(sourceUpdated(*newSource));
            oldSource = *newSource;
        }
    }
}

std::array<std::string, 512> MergeReceiver::getOwnerCids(const SacnRecvMergedData &mergedData)
{
    // Get source CIDs.
    std::unordered_map<sacn_remote_source_t, std::string> handleCids;
    handleCids.reserve(sources_.size());
    for (const auto &source : sources_ | std::views::values) {
        handleCids.emplace(source.handle, source.cid.ToString());
    }

    // Create a list of CIDs ordered by address.
    std::array<std::string, DMX_ADDRESS_COUNT> ownerCids{};
    const auto bufOffset = mergedData.slot_range.start_address - 1;
    for (std::size_t ix = 0; ix < mergedData.slot_range.address_count; ++ix) {
        const auto address = bufOffset + ix;
        Q_ASSERT(address < ownerCids.size());
        const auto ownerHandle = mergedData.owners[ix];
        ownerCids[address] = handleCids[ownerHandle];
    }

    return ownerCids;
}

} // namespace mobilesacn::handler
