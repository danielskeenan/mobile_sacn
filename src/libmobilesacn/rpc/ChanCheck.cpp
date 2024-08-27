/**
 * @file ChanCheckHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/ChanCheck.h>
#include <mobilesacn_messages/ChanCheck.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::rpc {

void ChanCheck::handleBinaryMessage(BinaryMessage data)
{
    auto msg = message::GetChanCheck(data.data());
    if (msg->val_type() == message::ChanCheckVal::transmit) {
        transmitting_ = msg->val_as_transmit()->transmit();
        spdlog::info("Transmit: {}", transmitting_);
    }
}

} // mobilesacn::rpc
