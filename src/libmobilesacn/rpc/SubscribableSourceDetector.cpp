/**
 * @file SubscribableSourceDetector.cpp
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/SubscribableSourceDetector.h>
#include <ranges>
#include <libmobilesacn/util.h>
#include <mobilesacn_messages/ReceiveLevelsResp.h>
#include <spdlog/spdlog.h>

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

bool SubscribableSourceDetector::startup(WsBinarySender* sender)
{
    spdlog::debug("Starting SourceDetector");
    const auto wsUserData = sender->getWsUserData();
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

void SubscribableSourceDetector::onSenderAdded(WsBinarySender* sender)
{
    // Send the new sender information about current sources.
    std::scoped_lock sourcesLock(sourcesMutex_);
    for (const auto& source : sources_ | std::views::values) {
        sendSourceUpdated(source, { sender });
    }
}

void SubscribableSourceDetector::HandleSourceUpdated(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name,
    const std::vector<uint16_t>& sourcedUniverses)
{
    std::scoped_lock lock(sendersMutex_, sourcesMutex_);
    auto& source = sources_[cid];
    source.cid = cid.ToString();
    source.name = name;
    source.universes = sourcedUniverses;
    spdlog::debug("Source {} ({}) updated with univs {}", source.cid, source.name,
                  fmt::join(sourcedUniverses, ", "));
    sendSourceUpdated(source, senders_);
}

void SubscribableSourceDetector::HandleSourceExpired(
    sacn::RemoteSourceHandle handle, const etcpal::Uuid& cid, const std::string& name)
{
    std::scoped_lock lock(sendersMutex_, sourcesMutex_);
    sources_.erase(cid);
    spdlog::debug("Source {} ({}) expired", cid.ToString(), name);
    sendSourceExpired(cid.ToString(), senders_);
}

void SubscribableSourceDetector::sendSourceUpdated(
    const Source& source, const SendersList& senders)
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
        builder,
        getNowInMilliseconds(),
        message::ReceiveLevelsRespVal::sourceUpdated,
        msgSourceUpdated.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendToSenders(senders, builder.GetBufferPointer(), builder.GetSize());
}

void SubscribableSourceDetector::sendSourceExpired(
    const std::string& cid, const SendersList& senders)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(cid);
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
    sendToSenders(senders, builder.GetBufferPointer(), builder.GetSize());
}

} // mobilesacn::rpc
