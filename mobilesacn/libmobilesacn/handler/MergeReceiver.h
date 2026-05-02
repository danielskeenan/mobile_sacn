/**
 * @file MergeReceiver.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_MERGERECEIVER_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_MERGERECEIVER_H

#include <sacn/cpp/merge_receiver.h>
#include <sacn/merge_receiver.h>
#include <QObject>

namespace mobilesacn::handler {

/**
 * Wrap an sacn::MergeReceiver to handle multiple subscribers.
 */
class MergeReceiver : public QObject, public sacn::MergeReceiver::NotifyHandler
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<MergeReceiver>;

    static Ptr getForUniverse(uint16_t universe);

    MergeReceiver(const MergeReceiver &) = delete;
    MergeReceiver &operator=(const MergeReceiver &) = delete;
    ~MergeReceiver() override;

    void HandleMergedData(
        sacn::MergeReceiver::Handle handle, const SacnRecvMergedData &merged_data) override;
    void HandleSourcesLost(
        sacn::MergeReceiver::Handle handle,
        uint16_t universe,
        const std::vector<SacnLostSource> &lostSources) override;

    void startup();
    void shutdown();

    [[nodiscard]] std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources() const;

Q_SIGNALS:
    void dataChanged(
        const SacnRecvMergedData &merged_data,
        const std::array<std::string, DMX_ADDRESS_COUNT> &ownerCids);
    void sourceUpdated(const sacn::MergeReceiver::Source &source);
    void sourceLost(const std::string &cid);

private:
    static inline std::mutex receiversMutex_;
    static inline std::unordered_map<uint16_t, std::weak_ptr<MergeReceiver>> receivers_;
    sacn::MergeReceiver::Settings sacnSettings_;
    sacn::MergeReceiver receiver_;
    mutable std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, sacn::MergeReceiver::Source> sources_;

    using QObject::QObject;

    void updateSources(const SacnRecvMergedData &mergedData);
    std::array<std::string, DMX_ADDRESS_COUNT> getOwnerCids(const SacnRecvMergedData &mergedData);
};

} // namespace mobilesacn::handler

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_MERGERECEIVER_H
