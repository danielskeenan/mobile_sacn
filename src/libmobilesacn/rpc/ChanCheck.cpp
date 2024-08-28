/**
 * @file ChanCheck.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <qcoreapplication.h>
#include <libmobilesacn/rpc/ChanCheck.h>
#include <mobilesacn_messages/ChanCheck.h>
#include <libmobilesacn/SacnCidGenerator.h>

namespace mobilesacn::rpc {

ChanCheck::ChanCheck(crow::websocket::connection& ws, QObject* parent)
    : RpcHandler(ws, parent)
{
    // Setup sACN.
    sacnSettings_.cid = SacnCidGenerator::get().cidForProtocolAndClient(
        kProtocol, ws.get_remote_ip());
    sacnSettings_.name = tr("%1 (Chan Check)").arg(qApp->applicationName()).toStdString();
    // Channel check only works on one universe at a time.
    sacnSettings_.universe_count_max = 1;
    univSettings_.universe = 1;
    univSettings_.priority = 100;
    updateLevelBuf();
    updatePapBuf();
}

ChanCheck::~ChanCheck()
{
    if (currentlyTransmitting()) {
        sacn_.Shutdown();
    }
}

void ChanCheck::handleBinaryMessage(BinaryMessage data)
{
    auto msg = message::GetChanCheck(data.data());
    if (msg->val_type() == message::ChanCheckVal::transmit) {
        onChangeTransmit(msg->val_as_transmit()->transmit());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    } else if (msg->val_type() == message::ChanCheckVal::priority) {
        onChangePriority(msg->val_as_priority()->priority());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    } else if (msg->val_type() == message::ChanCheckVal::perAddressPriority) {
        onChangePap(msg->val_as_perAddressPriority()->usePap());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    } else if (msg->val_type() == message::ChanCheckVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    } else if (msg->val_type() == message::ChanCheckVal::address) {
        onChangeAddress(msg->val_as_address()->address());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    } else if (msg->val_type() == message::ChanCheckVal::level) {
        onChangeLevel(msg->val_as_level()->level());
        ws_.send_binary(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    }
}

void ChanCheck::onChangeTransmit(bool transmit)
{
    if (currentlyTransmitting() == transmit) {
        return;
    }
    if (transmit) {
        sacn_.Startup(sacnSettings_);
        // The order these operations happen in is important!
        onChangeUniverse(univSettings_.universe);
        onChangeAddress(address_);
        onChangePriority(univSettings_.priority);
        onChangeLevel(level_);
        onChangePap(perAddressPriority_);
    } else {
        sacn_.Shutdown();
    }
}

void ChanCheck::onChangePriority(uint8_t priority)
{
    univSettings_.priority = priority;

    if (currentlyTransmitting()) {
        sacn_.ChangePriority(univSettings_.universe, univSettings_.priority);
    }
    updatePapBuf();
    sendLevelsAndPap();
}

void ChanCheck::onChangePap(bool usePap)
{
    perAddressPriority_ = usePap;

    sendLevelsAndPap();
}

void ChanCheck::onChangeUniverse(uint16_t useUniverse)
{
    univSettings_.universe = useUniverse;

    if (currentlyTransmitting()) {
        const auto currentUniverses = sacn_.GetUniverses();
        for (const auto universe : currentUniverses) {
            sacn_.RemoveUniverse(universe);
        }
        sacn_.AddUniverse(univSettings_.universe, getSacnMcastInterfaces());
    }
    sendLevelsAndPap();
}

void ChanCheck::onChangeAddress(uint16_t useAddress)
{
    address_ = useAddress;

    updateLevelBuf();
    updatePapBuf();
    sendLevelsAndPap();
}

void ChanCheck::onChangeLevel(uint8_t useLevel)
{
    level_ = useLevel;
    updateLevelBuf();
    sendLevelsAndPap();
}

void ChanCheck::updateLevelBuf()
{
    levelBuf_.fill(0);
    if (address_ > 0 && address_ <= levelBuf_.size()) {
        levelBuf_[address_ - 1] = level_;
    }
}

void ChanCheck::updatePapBuf()
{
    papBuf_.fill(0);
    if (address_ > 0 && address_ <= levelBuf_.size()) {
        papBuf_[address_ - 1] = univSettings_.priority;
    }
}

void ChanCheck::sendLevelsAndPap()
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
