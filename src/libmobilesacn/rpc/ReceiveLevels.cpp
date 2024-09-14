/**
 * @file ReceiveLevels.cpp
 *
 * @author Dan Keenan
 * @date 8/30/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/ReceiveLevels.h>
#include <mobilesacn_messages/ReceiveLevelsResp.h>
#include <mobilesacn_messages/ReceiveLevelsReq.h>
#include <flatbuffers/flatbuffers.h>
#include <libmobilesacn/util.h>
#include "libmobilesacn/rpc/SubscribableSourceDetector.h"

namespace mobilesacn::rpc {

ReceiveLevels::~ReceiveLevels()
{
    SubscribableSourceDetector::get().removeSender(this);
    if (receiver_) {
        receiver_->removeSender(this);
    }
}

void ReceiveLevels::handleConnected()
{
    // Send the current timestamp so the client can calibrate its offset relative to the server.
    flatbuffers::FlatBufferBuilder builder;
    const auto now = getNowInMilliseconds();
    auto msgSystemTime = message::CreateSystemTime(builder);
    auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder,
        now,
        message::ReceiveLevelsRespVal::systemTime,
        msgSystemTime.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendBinary(builder.GetBufferPointer(), builder.GetSize());

    SubscribableSourceDetector::get().addSender(this);
}

void ReceiveLevels::handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data)
{
    auto msg = message::GetReceiveLevelsReq(data.data());
    if (msg->val_type() == message::ReceiveLevelsReqVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
    }
}

void ReceiveLevels::handleClose()
{
    SubscribableSourceDetector::get().removeSender(this);
    if (receiver_) {
        receiver_->removeSender(this);
    }
}

void ReceiveLevels::onChangeUniverse(uint16_t universe)
{
    if (receiver_) {
        receiver_->removeSender(this);
    }
    if (universe > 0) {
        receiver_ = SubscribableMergeReceiver::getForUniverse(universe);
        receiver_->addSender(this);
    } else {
        receiver_.reset();
    }
}

} // mobilesacn::rpc
