/**
 * @file ReceiveLevels.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "ReceiveLevels.h"
#include "mobilesacn/libmobilesacn/util.h"
#include "mobilesacn_messages/LevelBuffer.h"
#include "mobilesacn_messages/ReceiveLevelsReq.h"
#include "mobilesacn_messages/ReceiveLevelsResp.h"
#include <mutex>
#include <ranges>
#include <spdlog/spdlog.h>

namespace mobilesacn::handler {

ReceiveLevels::ReceiveLevels(QWebSocket *ws, QObject *parent) : BaseHandler(ws, parent)
{
    connect(ws, &QWebSocket::binaryMessageReceived, this, &ReceiveLevels::onBinaryMessage);

    // Send the current timestamp so the client can calibrate its offset relative to the server.
    flatbuffers::FlatBufferBuilder builder;
    const auto now = getNowInMilliseconds();
    auto msgSystemTime = message::CreateSystemTime(builder);
    auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
        builder, now, message::ReceiveLevelsRespVal::systemTime, msgSystemTime.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());

    connect(
        SourceDetector::get(),
        &SourceDetector::sourceUpdated,
        this,
        qOverload<const SourceDetectorSource &>(&ReceiveLevels::onSourceUpdated));
    connect(
        SourceDetector::get(),
        &SourceDetector::sourceExpired,
        this,
        &ReceiveLevels::onSourceExpired);
    // Send known sources.
    for (const auto &source : SourceDetector::get()->sources() | std::views::values) {
        onSourceUpdated(source);
    }
}

void ReceiveLevels::onChangeUniverse(uint16_t universe)
{
    std::scoped_lock lastSeenLock(lastSeenMutex_);
    if (universe > 0) {
        receiver_ = MergeReceiver::getForUniverse(universe);
        connect(
            receiver_.get(),
            &MergeReceiver::sourceUpdated,
            this,
            qOverload<const sacn::MergeReceiver::Source &>(&ReceiveLevels::onSourceUpdated));
        connect(receiver_.get(), &MergeReceiver::dataChanged, this, &ReceiveLevels::onMergedData);
        connect(receiver_.get(), &MergeReceiver::sourceLost, this, &ReceiveLevels::onSourceLost);
        // Send known sources.
        for (const auto &source : receiver_->sources() | std::views::values) {
            onSourceUpdated(source);
        }
    } else {
        receiver_.reset();
    }
    lastSeen_.levels.fill(0);
    lastSeen_.priorities.fill(0);
    lastSeen_.owners.fill({});
}

void ReceiveLevels::onChangeFlickerFinder(bool flickerFinder)
{
    if (!flickerFinder_ && flickerFinder) {
        // Set up the flicker finder reference buffer.
        std::scoped_lock lock(flickerFinderReferenceBufferMutex_);
        flickerFinderReferenceBuffer_ = lastSeen_.levels;
    }
    flickerFinder_ = flickerFinder;
}

void ReceiveLevels::onBinaryMessage(const QByteArray &data)
{
    auto msg = message::GetReceiveLevelsReq(data.data());
    if (msg->val_type() == message::ReceiveLevelsReqVal::universe) {
        onChangeUniverse(msg->val_as_universe()->universe());
    } else if (msg->val_type() == message::ReceiveLevelsReqVal::flicker_finder) {
        onChangeFlickerFinder(msg->val_as_flicker_finder()->flickerFinder());
    }
}

void ReceiveLevels::onSourceUpdated(const SourceDetectorSource &source) const
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
        msgSourceUpdated.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onSourceUpdated(const sacn::MergeReceiver::Source &source) const
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
        msgSourceUpdated.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onMergedData(
    const SacnRecvMergedData &mergedData, const std::array<std::string, 512> &ownerCids)
{
    std::unique_lock<decltype(lastSeenMutex_)> lastSeenLock;
    if (flickerFinder_) {
        // Block for the lock, as we don't want to miss frames in flicker finder.
        lastSeenLock = std::unique_lock(lastSeenMutex_);
    } else {
        // If we can't get the lock, we will try again on the next frame.
        lastSeenLock = std::unique_lock(lastSeenMutex_, std::try_to_lock);
    }
    if (!lastSeenLock) {
        SPDLOG_DEBUG("Could not lock last seen buffers.");
        return;
    }

    // Determine where in addresses 1-512 our received data is.
    const auto bufOffset = mergedData.slot_range.start_address - 1;
    const auto bufCount = std::min(mergedData.slot_range.address_count, DMX_ADDRESS_COUNT);

    if (!flickerFinder_) {
        // Normal "display current levels" mode.
        std::memcpy(lastSeen_.levels.data() + bufOffset, mergedData.levels, bufCount);
        std::memcpy(lastSeen_.priorities.data() + bufOffset, mergedData.priorities, bufCount);
        lastSeen_.owners = ownerCids;

        flatbuffers::FlatBufferBuilder builder;

        const auto msgLevels = message::LevelBuffer(lastSeen_.levels);
        const auto msgPriorities = message::LevelBuffer(lastSeen_.priorities);
        const auto msgOwners = builder.CreateVectorOfStrings(ownerCids.cbegin(), ownerCids.cend());

        // Wrap the message.
        auto levelsChangedBuilder = message::LevelsChangedBuilder(builder);
        levelsChangedBuilder.add_levels(&msgLevels);
        levelsChangedBuilder.add_priorities(&msgPriorities);
        levelsChangedBuilder.add_owners(msgOwners);
        const auto msgLevelsChanged = levelsChangedBuilder.Finish();

        // Send the message.
        const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
            builder,
            getNowInMilliseconds(),
            message::ReceiveLevelsRespVal::levelsChanged,
            msgLevelsChanged.Union());
        builder.Finish(msgReceiveLevelsResp);
        sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
    } else {
        // Flicker finder mode.
        std::scoped_lock flickerFinderLock(flickerFinderReferenceBufferMutex_);
        decltype(lastSeen_.levels) levelsBuffer{};
        std::memcpy(levelsBuffer.data() + bufOffset, mergedData.levels, bufCount);
        // Compare new levels to levels stored in the buffer.
        std::vector<message::LevelChange> levelChanges;
        for (unsigned int address = 0; address < levelsBuffer.size(); ++address) {
            const auto oldLevel = lastSeen_.levels[address];
            const auto newLevel = levelsBuffer[address];
            if (newLevel != oldLevel) {
                // Found a flicker, add it to the list.
                const auto diff = newLevel - flickerFinderReferenceBuffer_[address];
                levelChanges.emplace_back(address, newLevel, diff);
            }
        }
        if (!levelChanges.empty()) {
            // Send flickers.
            flatbuffers::FlatBufferBuilder builder;
            const auto msgLevelChanges = builder.CreateVectorOfStructs(levelChanges);
            const auto msgFlicker = message::CreateFlicker(builder, msgLevelChanges);
            const auto msgReceiveLevelsResp = message::CreateReceiveLevelsResp(
                builder,
                getNowInMilliseconds(),
                message::ReceiveLevelsRespVal::flicker,
                msgFlicker.Union());
            builder.Finish(msgReceiveLevelsResp);
            sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
        }
        // Now that we've made comparisons, it's safe to update last seen.
        std::memcpy(lastSeen_.levels.data() + bufOffset, mergedData.levels, bufCount);
        std::memcpy(lastSeen_.priorities.data() + bufOffset, mergedData.priorities, bufCount);
        lastSeen_.owners = ownerCids;
    }
}

void ReceiveLevels::onSourceExpired(const std::string &cid) const
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
        msgSourceExpired.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
}

void ReceiveLevels::onSourceLost(const std::string &cid) const
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
        msgSourceExpired.Union());
    builder.Finish(msgReceiveLevelsResp);
    sendBinaryMessage(builder.GetBufferPointer(), builder.GetSize());
}

} // namespace mobilesacn::handler
