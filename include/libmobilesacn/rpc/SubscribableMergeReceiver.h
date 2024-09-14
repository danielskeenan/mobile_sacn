/**
 * @file SubscribableMergeReceiver.h
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#ifndef SUBSCRIBABLEMERGERECEIVER_H
#define SUBSCRIBABLEMERGERECEIVER_H

#include <sacn/cpp/merge_receiver.h>
#include "SubscribableNotifyHandler.h"

namespace mobilesacn::rpc {

/**
 * Wrap a MergeReceiver to handle multiple subscribers.
 */
class SubscribableMergeReceiver final : public SubscribableNotifyHandler,
                                        public sacn::MergeReceiver::NotifyHandler
{
public:
    using Ptr = std::shared_ptr<SubscribableMergeReceiver>;

    static Ptr getForUniverse(uint16_t universe);
    SubscribableMergeReceiver(const SubscribableMergeReceiver&) = delete;
    SubscribableMergeReceiver& operator=(const SubscribableMergeReceiver&) = delete;
    ~SubscribableMergeReceiver() override;

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

    explicit SubscribableMergeReceiver() = default;

    void updateSources(const SacnRecvMergedData& mergedData);
    static void sendSourceUpdated(const sacn::MergeReceiver::Source& source,
                                  const SendersList& senders);
    std::vector<std::string> getOwnerCids(const SacnRecvMergedData& mergedData);
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLEMERGERECEIVER_H
