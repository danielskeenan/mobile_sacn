/**
 * @file SubscribableMergeReceiver.cpp
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/SubscribableMergeReceiver.h>
#include <ranges>
#include <mobilesacn_messages/ReceiveLevelsResp.h>
#include <spdlog/spdlog.h>
#include "libmobilesacn/SacnSettings.h"

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
    const auto ownerCids = getOwnerCids(merged_data);
    sigMergedData_(merged_data, ownerCids);
}

void SubscribableMergeReceiver::HandleSourcesLost(sacn::MergeReceiver::Handle handle,
                                                  uint16_t universe,
                                                  const std::vector<SacnLostSource>& lostSources)
{
    for (const auto& source : lostSources) {
        const auto cid = etcpal::Uuid(source.cid);
        sigSourceLost_(cid);
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
    // Update sources.
    for (std::size_t ix = 0; ix < mergedData.num_active_sources; ++ix) {
        const auto newSource = receiver_.GetSource(mergedData.active_sources[ix]);
        Q_ASSERT(newSource);
        const auto cid = newSource->cid;
        auto& oldSource = sources_[cid];
        if (oldSource != *newSource) {
            sigSourceUpdated_(*newSource);
            oldSource = *newSource;
        }
    }
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

bool SubscribableMergeReceiver::startup(SubscriberPtr subscriber)
{
    spdlog::debug("Creating sACN Receiver for univ {}", sacnSettings_.universe_id);
    const auto& sacnSettings = SacnSettings::get();
    sacnSettings_.ip_supported = sacnSettings->sacnNetInt.addr().IsV4()
            ? sacn_ip_support_t::kSacnIpV4Only
            : sacn_ip_support_t::kSacnIpV6Only;
    auto mcastInterfaces = sacnSettings->sacnMcastInterfaces;
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

void SubscribableMergeReceiver::connectSignals(SubscriberPtr& subscriber)
{
    auto& conns = subscriberConnections_[subscriber];
    conns.emplace_back(sigMergedData_.connect(
            decltype(sigMergedData_)::slot_type(
                &MergeReceiverSubscriber::onMergedData,
                subscriber.get(),
                boost::placeholders::_1,
                boost::placeholders::_2
            )
            .track_foreign(subscriber))
    );
    conns.emplace_back(sigSourceUpdated_.connect(
            decltype(sigSourceUpdated_)::slot_type(
                &MergeReceiverSubscriber::onSourceUpdated,
                subscriber.get(),
                boost::placeholders::_1
            )
            .track_foreign(subscriber))
    );
    conns.emplace_back(sigSourceLost_.connect(
            decltype(sigSourceLost_)::slot_type(
                &MergeReceiverSubscriber::onSourceLost,
                subscriber.get(),
                boost::placeholders::_1
            )
            .track_foreign(subscriber))
    );

    // Send the new sender information about current sources.
    std::scoped_lock sourcesLock(sourcesMutex_);
    for (const auto& source : sources_ | std::views::values) {
        subscriber->onSourceUpdated(source);
    }
}

}
