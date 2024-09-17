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
    } else if (msg->val_type() == message::ReceiveLevelsReqVal::flicker_finder) {
        onChangeFlickerFinder(msg->val_as_flicker_finder()->flickerFinder());
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

void ReceiveLevels::onChangeFlickerFinder(bool flickerFinder)
{
    flickerFinder_ = flickerFinder;
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

template <typename T>
bool buffersEqual(std::span<T> lhs, T* rhs, std::size_t rhsSize)
{
    if (lhs.size() != rhsSize) {
        return false;
    }
    for (std::size_t ix = 0; ix < rhsSize; ++ix) {
        if (lhs[ix] != rhs[ix]) {
            return false;
        }
    }
    return true;
}

void ReceiveLevels::onMergedData(const SacnRecvMergedData& mergedData,
                                 const std::array<std::string, DMX_ADDRESS_COUNT>& ownerCids)
{
    const auto now = std::chrono::steady_clock::now();
    if (!flickerFinder_ && now - lastSent_ < kMessageInterval) {
        // Don't flood clients with messages, unless we are in flicker finder as we don't want
        // to miss flicker that last only one frame.
        return;
    }

    std::unique_lock<decltype(lastSeenMutex_)> lastSeenLock;
    if (flickerFinder_) {
        // Block for the lock, as we don't want to miss frames in flicker finder.
        lastSeenLock = std::unique_lock(lastSeenMutex_);
    } else {
        // If we can't get the lock, we will try again on the next frame.
        lastSeenLock = std::unique_lock(lastSeenMutex_, std::try_to_lock);
    }
    if (!lastSeenLock) {
        spdlog::debug("Could not lock last seen buffers.");
        return;
    }

    // Determine where in addresses 1-512 our received data is.
    const auto bufOffset = mergedData.slot_range.start_address - 1;
    const auto bufCount = std::min(mergedData.slot_range.address_count, DMX_ADDRESS_COUNT);
    // Determine if the data has changed since the last message.
    if (buffersEqual(std::span<const uint8_t>(lastSeen_.levels).subspan(bufOffset, bufCount),
                     mergedData.levels, bufCount)
        && buffersEqual(std::span<const uint8_t>(lastSeen_.priorities).subspan(bufOffset, bufCount),
                        mergedData.priorities, bufCount)
        && lastSeen_.owners == ownerCids) {
        // Nothing has changed, send no messages.
        return;
    }
    // Something has changed, send a message.

    if (!flickerFinder_) {
        // Normal "display current levels" mode.
        std::memcpy(lastSeen_.levels.data() + bufOffset, mergedData.levels, bufCount);
        std::memcpy(lastSeen_.priorities.data() + bufOffset, mergedData.priorities, bufCount);
        lastSeen_.owners = ownerCids;

        flatbuffers::FlatBufferBuilder builder;

        auto msgLevels = message::LevelBuffer(lastSeen_.levels);
        auto msgPriorities = message::LevelBuffer(lastSeen_.priorities);
        auto msgOwners = builder.CreateVectorOfStrings(ownerCids.cbegin(), ownerCids.cend());

        // Wrap the message.
        auto levelsChangedBuilder = message::LevelsChangedBuilder(builder);
        levelsChangedBuilder.add_levels(&msgLevels);
        levelsChangedBuilder.add_priorities(&msgPriorities);
        levelsChangedBuilder.add_owners(msgOwners);
        auto msgLevelsChanged = levelsChangedBuilder.Finish();

        // Send the message.
        const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
            builder, getNowInMilliseconds(), message::ReceiveLevelsRespVal::levelsChanged,
            msgLevelsChanged.Union());
        builder.Finish(msgReceiveLevelsResp);
        sendBinary(builder.GetBufferPointer(), builder.GetSize());
    } else {
        // Flicker finder mode.
        decltype(lastSeen_.levels) levelsBuffer{};
        std::memcpy(levelsBuffer.data() + bufOffset, mergedData.levels, bufCount);
        // Compare new levels to levels stored in the buffer.
        std::vector<message::LevelChange> levelChanges;
        for (unsigned int address = 0; address < levelsBuffer.size(); ++address) {
            const auto oldLevel = lastSeen_.levels[address];
            const auto newLevel = levelsBuffer[address];
            const int diff = newLevel - oldLevel;
            if (diff != 0) {
                // Found a flicker, add it to the list.
                levelChanges.emplace_back(address, newLevel, diff);
            }
        }
        if (!levelChanges.empty()) {
            // Send flickers.
            flatbuffers::FlatBufferBuilder builder;
            const auto msgLevelChanges = builder.CreateVectorOfStructs(levelChanges);
            const auto msgFlicker = message::CreateFlicker(builder, msgLevelChanges);
            const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
                builder, getNowInMilliseconds(), message::ReceiveLevelsRespVal::flicker,
                msgFlicker.Union());
            builder.Finish(msgReceiveLevelsResp);
            sendBinary(builder.GetBufferPointer(), builder.GetSize());
        }
        // Now that we've made comparisons, it's safe to update last seen.
        std::memcpy(lastSeen_.levels.data() + bufOffset, mergedData.levels, bufCount);
        std::memcpy(lastSeen_.priorities.data() + bufOffset, mergedData.priorities, bufCount);
        lastSeen_.owners = ownerCids;
    }
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
