/**
 * @file SourceDetector.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "SourceDetector.h"
#include "mobilesacn/libmobilesacn/SacnSettings.h"
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::handler {

SourceDetector::~SourceDetector()
{
    shutdown();
}

SourceDetector *SourceDetector::get()
{
    static SourceDetector *instance = []() { return new SourceDetector; }();
    return instance;
}

void SourceDetector::startup()
{
    SPDLOG_DEBUG("Starting SourceDetector");
    const auto &sacnSettings = SacnSettings::get();
    auto settings = sacn::SourceDetector::Settings();
    // Creating the receiver will fail with the default settings if the chosen network interface
    // doesn't have an IPv4 AND an IPv6 address.
    settings.ip_supported = sacnSettings->sacnNetInt.addr().IsV4()
                                ? sacn_ip_support_t::kSacnIpV4Only
                                : sacn_ip_support_t::kSacnIpV6Only;
    auto mcastInterfaces = sacnSettings->sacnMcastInterfaces;
    auto res = sacn::SourceDetector::Startup(settings, *this, mcastInterfaces);
    if (res != kEtcPalErrOk) {
        SPDLOG_ERROR("Failed to start SourceDetector: {}", res.ToString());
    }
}

void SourceDetector::shutdown()
{
    std::scoped_lock sourcesLock(sourcesMutex_);
    SPDLOG_DEBUG("Stopping SourceDetector");
    sacn::SourceDetector::Shutdown();
    sources_.clear();
}

std::unordered_map<etcpal::Uuid, SourceDetectorSource> SourceDetector::sources() const
{
    std::scoped_lock sourcesLock(sourcesMutex_);
    // Return a copy to avoid threading issues.
    return {sources_};
}

void SourceDetector::HandleSourceUpdated(
    sacn::RemoteSourceHandle handle,
    const etcpal::Uuid &cid,
    const std::string &name,
    const std::vector<uint16_t> &sourcedUniverses)
{
    std::scoped_lock sourcesLock(sourcesMutex_);

    auto &source = sources_[cid];
    source.cid = cid.ToString();
    source.name = name;
    source.universes = sourcedUniverses;
    SPDLOG_DEBUG(
        "Source {} ({}) updated with univs {}",
        source.cid,
        source.name,
        fmt::join(sourcedUniverses, ", "));
    Q_EMIT(sourceUpdated(source));
}

void SourceDetector::HandleSourceExpired(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid &cid, const std::string &name)
{
    std::scoped_lock sourcesLock(sourcesMutex_);

    sources_.erase(cid);
    SPDLOG_DEBUG("Source {} ({}) expired", cid.ToString(), name);
    Q_EMIT(sourceExpired(cid.ToString()));
}

} // namespace mobilesacn::handler
