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

    SubscribableSourceDetector::get().subscribe(shared_from_this());
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
    SubscribableSourceDetector::get().unsubscribe(shared_from_this());
    if (receiver_) {
        receiver_->unsubscribe(shared_from_this());
        receiver_.reset();
    }
}

void ReceiveLevels::onChangeUniverse(uint16_t universe)
{
    if (receiver_) {
        receiver_->unsubscribe(shared_from_this());
    }

    if (universe > 0) {
        receiver_ = SubscribableMergeReceiver::getForUniverse(universe);
        receiver_->subscribe(shared_from_this());
    } else {
        receiver_.reset();
    }
}

void ReceiveLevels::onSourceUpdated(const SourceDetectorSource& source)
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
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onSourceUpdated(const sacn::MergeReceiver::Source& source)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(source.cid.ToString());
    const auto msgName = builder.CreateString(source.name);
    const auto msgIpAddr = builder.CreateString(source.addr.ip().ToString());
    auto sourceUpdatedBuilder = message::SourceUpdatedBuilder(builder);
    sourceUpdatedBuilder.add_cid(msgCid);
    sourceUpdatedBuilder.add_name(msgName);
    sourceUpdatedBuilder.add_ipAddr(msgIpAddr);
    sourceUpdatedBuilder.add_hasPap(source.per_address_priorities_active);
    sourceUpdatedBuilder.add_priority(source.universe_priority);
    const auto msgSourceUpdated = sourceUpdatedBuilder.Finish();
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder,
        getNowInMilliseconds(),
        message::ReceiveLevelsRespVal::sourceUpdated,
        msgSourceUpdated.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onSourceExpired(const etcpal::Uuid& cid)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(cid.ToString());
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
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onMergedData(const SacnRecvMergedData& merged_data,
                                 const std::vector<std::string>& ownerCids)
{
    const auto now = std::chrono::steady_clock::now();
    if (now - lastSent_ < kMessageInterval) {
        // Don't flood clients with messages.
        return;
    }
    flatbuffers::FlatBufferBuilder builder;

    std::array<uint8_t, DMX_ADDRESS_COUNT> levelBuf{};
    const auto bufOffset = merged_data.slot_range.start_address - 1;
    const auto bufCount = std::min(merged_data.slot_range.address_count, DMX_ADDRESS_COUNT);

    // Levels
    std::memcpy(levelBuf.data() + bufOffset, merged_data.levels, bufCount);
    auto msgLevels = message::LevelBuffer(levelBuf);
    levelBuf.fill(0);

    // Priorities
    std::memcpy(levelBuf.data() + bufOffset, merged_data.priorities, bufCount);
    auto msgPriorities = message::LevelBuffer(levelBuf);

    // Owners
    auto msgOwners = builder.CreateVectorOfStrings(ownerCids);

    // Wrap the message.
    auto levelsChangedBuilder = message::LevelsChangedBuilder(builder);
    levelsChangedBuilder.add_levels(&msgLevels);
    levelsChangedBuilder.add_priorities(&msgPriorities);
    levelsChangedBuilder.add_owners(msgOwners);
    auto msgLevelsChanged = levelsChangedBuilder.Finish();

    // Send the message.
    const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder,
        getNowInMilliseconds(),
        message::ReceiveLevelsRespVal::levelsChanged,
        msgLevelsChanged.Union()
    );
    builder.Finish(msgReceiveLevelsResp);
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
    lastSent_ = now;
}

void ReceiveLevels::onSourceLost(const etcpal::Uuid& cid)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto msgCid = builder.CreateString(cid.ToString());
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
    sendBinary(builder.GetBufferPointer(), builder.GetSize());
}

} // mobilesacn::rpc
