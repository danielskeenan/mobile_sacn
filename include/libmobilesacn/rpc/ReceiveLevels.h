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

namespace mobilesacn::rpc {
class SourceDetectorWrapper final : public sacn::SourceDetector::NotifyHandler {
  public:
    static SourceDetectorWrapper &get();
    ~SourceDetectorWrapper() override;

    void addSender(WsBinarySender *sender);
    void removeSender(WsBinarySender *sender);

    void HandleSourceUpdated(sacn::RemoteSourceHandle handle,
                             const etcpal::Uuid &cid,
                             const std::string &name,
                             const std::vector<uint16_t> &sourcedUniverses) override;
    void HandleSourceExpired(sacn::RemoteSourceHandle handle,
                             const etcpal::Uuid &cid,
                             const std::string &name) override;

  private:
    struct Source {
      std::string cid;
      std::string name;
      std::vector<uint16_t> universes;
    };

    std::mutex sendersMutex_;
    std::vector<WsBinarySender *> senders_;
    std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, Source> sources_;

    static void sendSourceUpdated(const Source &source, const decltype(senders_) &senders);
    static void sendSourceExpired(const std::string &cid, const decltype(senders_) &senders);
    static void sendToSenders(const decltype(senders_) &senders,
                              const uint8_t *data,
                              std::size_t size);
    explicit SourceDetectorWrapper() = default;
    static void resetNetworkingIfNeeded(std::vector<SacnMcastInterface> &netints);
};

class ReceiveLevels final : public RpcHandler, public sacn::MergeReceiver::NotifyHandler {
    Q_OBJECT

  public:
    using RpcHandler::RpcHandler;
    ~ReceiveLevels() override;

    static constexpr auto kProtocol = "ReceiveLevels";
    [[nodiscard]] const char *getProtocol() const override { return kProtocol; }
    [[nodiscard]] QString getDisplayName() const override { return tr("Receive Levels"); }

  public Q_SLOTS:
    void handleConnected() override;
    void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) override;
    void handleClose() override;
    void HandleMergedData(sacn::MergeReceiver::Handle handle,
                          const SacnRecvMergedData &merged_data) override;

  private:
    sacn::MergeReceiver::Settings sacnSettings_;
    sacn::MergeReceiver receiver_;
    std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources_;

    void onChangeUniverse(uint16_t universe);
    void updateSources(const SacnRecvMergedData &mergedData);
    std::vector<std::string> getOwnerCids(const SacnRecvMergedData &mergedData);
};
} // mobilesacn::rpc

#endif //RECEIVELEVELS_H
