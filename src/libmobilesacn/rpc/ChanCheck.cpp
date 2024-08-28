/**
 * @file ChanCheck.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/ChanCheck.h>
#include <mobilesacn_messages/ChanCheck.h>
#include <QApplication>

namespace mobilesacn::rpc {

void ChanCheck::handleConnected()
{
    TransmitHandler::handleConnected();
    updateLevelBuf();
    updatePapBuf();
}

void ChanCheck::handleBinaryMessage(BinaryMessage data)
{
    auto msg = message::GetChanCheck(data.data());
    if (msg->val_type() == message::ChanCheckVal::transmit) {
        onChangeTransmit(msg->val_as_transmit()->transmit());
    } else if (msg->val_type() == message::ChanCheckVal::priority) {
        onChangePriority(msg->val_as_priority()->priority());
    } else if (msg->val_type() == message::ChanCheckVal::perAddressPriority) {
        onChangePap(msg->val_as_perAddressPriority()->usePap());
    } else if (msg->val_type() == message::ChanCheckVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
    } else if (msg->val_type() == message::ChanCheckVal::address) {
        onChangeAddress(msg->val_as_address()->address());
    } else if (msg->val_type() == message::ChanCheckVal::level) {
        onChangeLevel(msg->val_as_level()->level());
    }
}

void ChanCheck::startTransmitting()
{
    sacn_.Startup(sacnSettings_);
    // The order these operations happen in is important!
    onChangeUniverse(univSettings_.universe);
    onChangeAddress(address_);
    onChangePriority(univSettings_.priority);
    onChangeLevel(level_);
    onChangePap(perAddressPriority_);
    sendLevelsAndPap();
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

} // mobilesacn::rpc
