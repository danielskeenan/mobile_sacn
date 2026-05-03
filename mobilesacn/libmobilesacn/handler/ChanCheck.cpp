/**
 * @file ChanCheck.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "ChanCheck.h"
#include "mobilesacn_messages/ChanCheck.h"

namespace mobilesacn::handler {

ChanCheck::ChanCheck(QWebSocket *ws, QObject *parent) :
    TransmitHandler(ws, parent), blinker_(new QTimer(this))
{
    blinker_->setInterval(1000);
    blinker_->setTimerType(Qt::PreciseTimer);
    connect(blinker_, &QTimer::timeout, this, &ChanCheck::blink);

    updateLevelBuf();
    updatePapBuf();
}

void ChanCheck::onBinaryMessage(const QByteArray &message)
{
    auto msg = message::GetChanCheck(message.data());
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
    } else if (msg->val_type() == message::ChanCheckVal::blink) {
        onChangeBlink(msg->val_as_blink()->blink());
    }
}

void ChanCheck::startTransmitting()
{
    onChangeAddress(address_);
    onChangeLevel(level_);
    TransmitHandler::startTransmitting();
}

void ChanCheck::onChangePriority(const uint8_t priority)
{
    univSettings_.priority = priority;
    updatePapBuf();
    TransmitHandler::onChangePriority(priority);
}

void ChanCheck::onChangeAddress(const uint16_t useAddress)
{
    address_ = useAddress;

    updateLevelBuf();
    updatePapBuf();
    sendLevelsAndPap();
}

void ChanCheck::onChangeLevel(const uint8_t useLevel)
{
    level_ = useLevel;
    updateLevelBuf();
    sendLevelsAndPap();
}

void ChanCheck::onChangeBlink(const bool blink)
{
    if (blink) {
        blinker_->start();
    } else {
        blinker_->stop();
    }
}

void ChanCheck::blink()
{
    auto &currentLevel = levelBuf_[address_ - 1];
    if (currentLevel > 0) {
        currentLevel = 0;
    } else {
        currentLevel = level_;
    }
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

} // namespace mobilesacn::handler
