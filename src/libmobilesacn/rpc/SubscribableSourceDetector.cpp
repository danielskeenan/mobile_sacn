/**
 * @file SubscribableSourceDetector.cpp
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/SubscribableSourceDetector.h>
#include <ranges>
#include <spdlog/spdlog.h>

#include "libmobilesacn/SacnSettings.h"

namespace mobilesacn::rpc {

SubscribableSourceDetector::~SubscribableSourceDetector()
{
    sacn::SourceDetector::Shutdown();
}

SubscribableSourceDetector& SubscribableSourceDetector::get()
{
    static SubscribableSourceDetector instance;
    return instance;
}

bool SubscribableSourceDetector::startup(SubscriberPtr subscriber)
{
    spdlog::debug("Starting SourceDetector");
    const auto& sacnSettings = SacnSettings::get();
    auto settings = sacn::SourceDetector::Settings();
    // Creating the receiver will fail with the default settings if the chosen network interface
    // doesn't have an IPv4 AND an IPv6 address.
    settings.ip_supported = sacnSettings->sacnNetInt.addr().IsV4()
            ? sacn_ip_support_t::kSacnIpV4Only
            : sacn_ip_support_t::kSacnIpV6Only;
    auto mcastInterfaces = sacnSettings->sacnMcastInterfaces;
    auto res = sacn::SourceDetector::Startup(
        settings,
        *this,
        mcastInterfaces
    );
    if (res != kEtcPalErrOk) {
        spdlog::error("Failed to start SourceDetector: {}", res.ToString());
        return false;
    }

    return true;
}

void SubscribableSourceDetector::shutdown()
{
    std::scoped_lock sourcesLock(sourcesMutex_);
    spdlog::debug("Stopping SourceDetector");
    sacn::SourceDetector::Shutdown();
    sources_.clear();
}

void SubscribableSourceDetector::HandleSourceUpdated(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name,
    const std::vector<uint16_t>& sourcedUniverses)
{
    auto& source = sources_[cid];
    source.cid = cid.ToString();
    source.name = name;
    source.universes = sourcedUniverses;
    spdlog::debug("Source {} ({}) updated with univs {}", source.cid, source.name,
                  fmt::join(sourcedUniverses, ", "));
    sigSourceUpdated_(source);
}

void SubscribableSourceDetector::HandleSourceExpired(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name)
{
    sources_.erase(cid);
    spdlog::debug("Source {} ({}) expired", cid.ToString(), name);
    sigSourceExpired_(cid);
}

void SubscribableSourceDetector::connectSignals(SubscriberPtr& subscriber)
{
    auto& conns = subscriberConnections_[subscriber];
    conns.emplace_back(
        sigSourceUpdated_.connect(
            decltype(sigSourceUpdated_)::slot_type(
                &SourceDetectorSubscriber::onSourceUpdated,
                subscriber.get(),
                boost::placeholders::_1
            ).track_foreign(subscriber))
    );
    conns.emplace_back(
        sigSourceExpired_.connect(
            decltype(sigSourceExpired_)::slot_type(
                &SourceDetectorSubscriber::onSourceExpired,
                subscriber.get(),
                boost::placeholders::_1)
            .track_foreign(subscriber))
    );

    // Send the new sender information about current sources.
    std::scoped_lock sourcesLock(sourcesMutex_);
    for (const auto& source : sources_ | std::views::values) {
        subscriber->onSourceUpdated(source);
    }
}

} // mobilesacn::rpc
