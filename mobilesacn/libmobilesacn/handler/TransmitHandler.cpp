/**
 * @file TransmitHandler.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "TransmitHandler.h"
#include "mobilesacn/libmobilesacn/SacnCidGenerator.h"
#include "mobilesacn/libmobilesacn/SacnSettings.h"
#include <spdlog/spdlog.h>
#include <QApplication>

namespace mobilesacn::handler {

TransmitHandler::TransmitHandler(QWebSocket *ws, QObject *parent) : BaseHandler(ws, parent)
{
    univSettings_.universe = 1;
    univSettings_.priority = 100;
}

TransmitHandler::~TransmitHandler()
{
    if (currentlyTransmitting()) {
        sacn_.Shutdown();
    }
}

std::string TransmitHandler::getSourceName() const
{
    const auto clientIpAddr = ws()->peerAddress().toString();
    return tr("%1 (%2 %3)")
        .arg(qApp->applicationDisplayName(), clientIpAddr, getDisplayName())
        .toStdString();
}

void TransmitHandler::onChangeTransmit(const bool transmit)
{
    if (currentlyTransmitting() == transmit) {
        return;
    }
    if (transmit) {
        startTransmitting();
    } else {
        stopTransmitting();
    }
}

void TransmitHandler::onChangePriority(const uint8_t priority)
{
    univSettings_.priority = priority;

    if (currentlyTransmitting()) {
        sacn_.ChangePriority(univSettings_.universe, univSettings_.priority);
    }
    sendLevelsAndPap();
}

void TransmitHandler::onChangePap(const bool usePap)
{
    perAddressPriority_ = usePap;

    sendLevelsAndPap();
}

void TransmitHandler::onChangeUniverse(uint16_t useUniverse)
{
    univSettings_.universe = useUniverse;

    if (currentlyTransmitting()) {
        const auto currentUniverses = sacn_.GetUniverses();
        for (const auto universe : currentUniverses) {
            sacn_.RemoveUniverse(universe);
        }
        auto mcastInterfaces = SacnSettings::get()->sacnMcastInterfaces;
        sacn_.AddUniverse(univSettings_.universe, mcastInterfaces);
    }
    sendLevelsAndPap();
}

void TransmitHandler::startTransmitting()
{
    if (!sacnSettings_.IsValid()) {
        // Setup sACN.
        const auto clientIpAddr = ws()->peerAddress().toString().toStdString();
        sacnSettings_.cid
            = SacnCidGenerator::get().cidForProtocolAndClient(getProtocol(), clientIpAddr);
        sacnSettings_.name = getSourceName();
        sacnSettings_.universe_count_max = 1;
    }
    const auto res = sacn_.Startup(sacnSettings_);
    if (!res.IsOk()) {
        SPDLOG_CRITICAL("Error starting sACN Transmitter: {}", res.ToString());
        sacn_.Shutdown();
        return;
    }
    // The order these operations happen in is important!
    onChangeUniverse(univSettings_.universe);
    onChangePriority(univSettings_.priority);
    onChangePap(perAddressPriority_);
    sendLevelsAndPap();
}

void TransmitHandler::stopTransmitting()
{
    sacn_.Shutdown();
}

void TransmitHandler::sendLevelsAndPap()
{
    if (currentlyTransmitting()) {
        sacn_.UpdateLevelsAndPap(
            univSettings_.universe,
            levelBuf_.data(),
            levelBuf_.size(),
            perAddressPriority_ ? papBuf_.data() : nullptr,
            perAddressPriority_ ? papBuf_.size() : 0);
    }
}

} // namespace mobilesacn::handler
