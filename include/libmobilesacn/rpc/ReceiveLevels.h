/**
 * @file ReceiveLevels.h
 *
 * @author Dan Keenan
 * @date 8/30/24
 * @copyright GNU GPLv3
 */

#ifndef RECEIVELEVELS_H
#define RECEIVELEVELS_H

#include <set>

#include "RpcHandler.h"
#include <sacn/cpp/source_detector.h>
#include <sacn/cpp/merge_receiver.h>
#include "SubscribableNotifyHandler.h"

namespace mobilesacn::rpc {

/**
 * Wrap the SourceDetector to handle multiple subscribers.
 */
class SourceDetectorWrapper final : public SubscribableNotifyHandler,
                                    public sacn::SourceDetector::NotifyHandler
{
public:
    static SourceDetectorWrapper& get();

    SourceDetectorWrapper(const SourceDetectorWrapper&) = delete;
    SourceDetectorWrapper& operator=(const SourceDetectorWrapper&) = delete;
    ~SourceDetectorWrapper() override;

    void HandleSourceUpdated(sacn::RemoteSourceHandle handle,
                             const etcpal::Uuid& cid,
                             const std::string& name,
                             const std::vector<uint16_t>& sourcedUniverses) override;
    void HandleSourceExpired(sacn::RemoteSourceHandle handle,
                             const etcpal::Uuid& cid,
                             const std::string& name) override;

protected:
    bool startup(WsBinarySender* sender) override;
    void shutdown() override;
    void onSenderAdded(WsBinarySender* sender) override;

private:
    struct Source
    {
        std::string cid;
        std::string name;
        std::vector<uint16_t> universes;
    };

    std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, Source> sources_;

    explicit SourceDetectorWrapper() = default;

    static void sendSourceUpdated(const Source& source, const SendersList& senders);
    static void sendSourceExpired(const std::string& cid, const SendersList& senders);
};

/**
 * Wrap a MergeReceiver to handle multiple subscribers.
 */
class SacnReceiverWrapper final
        : public SubscribableNotifyHandler, public sacn::MergeReceiver::NotifyHandler
{
public:
    using Ptr = std::shared_ptr<SacnReceiverWrapper>;

    static Ptr getForUniverse(uint16_t universe);
    SacnReceiverWrapper(const SacnReceiverWrapper&) = delete;
    SacnReceiverWrapper& operator=(const SacnReceiverWrapper&) = delete;
    ~SacnReceiverWrapper() override;

    void HandleMergedData(sacn::MergeReceiver::Handle handle,
                          const SacnRecvMergedData& merged_data) override;
    void HandleSourcesLost(sacn::MergeReceiver::Handle handle, uint16_t universe,
                           const std::vector<SacnLostSource>& lostSources) override;

protected:
    bool startup(WsBinarySender* sender) override;
    void shutdown() override;
    void onSenderAdded(WsBinarySender* sender) override;

private:
    static inline std::mutex receiversMutex_;
    static inline std::unordered_map<uint16_t, Ptr> receivers_;

    sacn::MergeReceiver::Settings sacnSettings_;
    sacn::MergeReceiver receiver_;
    std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources_;

    explicit SacnReceiverWrapper() = default;

    void updateSources(const SacnRecvMergedData& mergedData);
    static void sendSourceUpdated(const sacn::MergeReceiver::Source& source, const SendersList& senders);
    std::vector<std::string> getOwnerCids(const SacnRecvMergedData& mergedData);
};

/**
 * Handler for Receive Levels.
 */
class ReceiveLevels final : public RpcHandler
{
    Q_OBJECT

public:
    using RpcHandler::RpcHandler;
    ~ReceiveLevels() override;

    static constexpr auto kProtocol = "ReceiveLevels";
    [[nodiscard]] const char* getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Receive Levels"); }

public Q_SLOTS:
    void handleConnected() override;
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;
    void handleClose() override;

private:
    SacnReceiverWrapper::Ptr receiver_;

    void onChangeUniverse(uint16_t universe);
};
} // mobilesacn::rpc

#endif //RECEIVELEVELS_H
