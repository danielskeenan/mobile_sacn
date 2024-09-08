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
 * Handler for Receive Levels.
 */
class ReceiveLevels final : public RpcHandler, public sacn::MergeReceiver::NotifyHandler
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
    void HandleMergedData(sacn::MergeReceiver::Handle handle,
                          const SacnRecvMergedData& merged_data) override;
    void HandleSourcesLost(sacn::MergeReceiver::Handle handle, uint16_t universe,
                           const std::vector<SacnLostSource>& lostSources) override;

private:
    sacn::MergeReceiver::Settings sacnSettings_;
    sacn::MergeReceiver receiver_;
    std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources_;

    void onChangeUniverse(uint16_t universe);
    void updateSources(const SacnRecvMergedData& mergedData);
    std::vector<std::string> getOwnerCids(const SacnRecvMergedData& mergedData);
};
} // mobilesacn::rpc

#endif //RECEIVELEVELS_H
