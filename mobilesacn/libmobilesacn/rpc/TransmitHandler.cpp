/**
 * @file TransmitHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/28/24
 * @copyright GNU GPLv3
 */

#include <QApplication>
#include "TransmitHandler.h"
#include "mobilesacn/libmobilesacn/SacnCidGenerator.h"
#include <spdlog/spdlog.h>
#include "mobilesacn/libmobilesacn/SacnSettings.h"

namespace mobilesacn::rpc {

TransmitHandler::~TransmitHandler()
{
    if (currentlyTransmitting()) {
        sacn_.Shutdown();
    }
}

std::string TransmitHandler::getSourceName() const
{
    const auto clientIpAddr = getWsUserData()->clientIp;
    return tr("%1 (%2 %3)")
            .arg(
                qApp->applicationDisplayName(),
                QString::fromStdString(clientIpAddr),
                getDisplayName()
            )
            .toStdString();
}

void TransmitHandler::handleConnected()
{
    const auto clientIpAddr = getWsUserData()->clientIp;
    // Setup sACN.
    sacnSettings_.cid = SacnCidGenerator::get().cidForProtocolAndClient(
        getProtocol(), clientIpAddr);
    sacnSettings_.name = getSourceName();
    // Channel check only works on one universe at a time.
    sacnSettings_.universe_count_max = 1;
    univSettings_.universe = 1;
    univSettings_.priority = 100;
}

void TransmitHandler::onChangeTransmit(bool transmit)
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

void TransmitHandler::onChangePriority(uint8_t priority)
{
    univSettings_.priority = priority;

    if (currentlyTransmitting()) {
        sacn_.ChangePriority(univSettings_.universe, univSettings_.priority);
    }
    sendLevelsAndPap();
}

void TransmitHandler::onChangePap(bool usePap)
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
    const auto res = sacn_.Startup(sacnSettings_);
    if (!res.IsOk()) {
        spdlog::critical("Error starting sACN Transmitter: {}", res.ToString());
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
            perAddressPriority_ ? papBuf_.size() : 0
        );
    }
}

} // mobilesacn::rpc
