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
#include <boost/signals2/signal.hpp>

namespace mobilesacn::rpc {

class MergeReceiverSubscriber
{
public:
    virtual void onMergedData(const SacnRecvMergedData& merged_data,
                              const std::array<std::string, DMX_ADDRESS_COUNT> &ownerCids) = 0;
    virtual void onSourceUpdated(const sacn::MergeReceiver::Source& source) = 0;
    virtual void onSourceLost(const etcpal::Uuid& cid) = 0;
};

/**
 * Wrap a MergeReceiver to handle multiple subscribers.
 */
class SubscribableMergeReceiver final : public SubscribableNotifyHandler<MergeReceiverSubscriber>,
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
    bool startup(SubscriberPtr subscriber) override;
    void shutdown() override;
    void connectSignals(SubscriberPtr& subscriber) override;

private:
    static inline std::mutex receiversMutex_;
    static inline std::unordered_map<uint16_t, Ptr> receivers_;

    sacn::MergeReceiver::Settings sacnSettings_;
    sacn::MergeReceiver receiver_;
    std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources_;
    boost::signals2::signal<void(const SacnRecvMergedData& merged_data,
                                 const std::array<std::string, DMX_ADDRESS_COUNT>& ownerCids)> sigMergedData_;
    boost::signals2::signal<void(const sacn::MergeReceiver::Source& source)> sigSourceUpdated_;
    boost::signals2::signal<void(const etcpal::Uuid& cid)> sigSourceLost_;

    explicit SubscribableMergeReceiver() = default;

    void updateSources(const SacnRecvMergedData& mergedData);
    std::array<std::string, DMX_ADDRESS_COUNT> getOwnerCids(const SacnRecvMergedData &mergedData);
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLEMERGERECEIVER_H
