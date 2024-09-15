/**
 * @file ReceiveLevels.h
 *
 * @author Dan Keenan
 * @date 8/30/24
 * @copyright GNU GPLv3
 */

#ifndef RECEIVELEVELS_H
#define RECEIVELEVELS_H

#include "RpcHandler.h"
#include <sacn/cpp/merge_receiver.h>
#include "SubscribableMergeReceiver.h"
#include "SubscribableSourceDetector.h"

namespace mobilesacn::rpc {

/**
 * Handler for Receive Levels.
 */
class ReceiveLevels final : public RpcHandler, public SourceDetectorSubscriber,
                            public MergeReceiverSubscriber,
                            public std::enable_shared_from_this<ReceiveLevels>
{
    Q_OBJECT

public:
    using RpcHandler::RpcHandler;
    ~ReceiveLevels() override = default;

    static constexpr auto kProtocol = "ReceiveLevels";
    [[nodiscard]] const char* getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Receive Levels"); }
    void onSourceUpdated(const SourceDetectorSource& source) override;
    void onSourceUpdated(const sacn::MergeReceiver::Source& source) override;
    void onSourceExpired(const etcpal::Uuid& cid) override;
    void onMergedData(const SacnRecvMergedData& merged_data,
                      const std::vector<std::string>& ownerCids) override;
    void onSourceLost(const etcpal::Uuid& cid) override;

public Q_SLOTS:
    void handleConnected() override;
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;
    void handleClose() override;

private:
    static constexpr auto kMessageInterval = std::chrono::milliseconds(100);
    SubscribableMergeReceiver::Ptr receiver_;
    std::chrono::steady_clock::time_point lastSent_;

    void onChangeUniverse(uint16_t universe);

};
} // mobilesacn::rpc

#endif //RECEIVELEVELS_H
