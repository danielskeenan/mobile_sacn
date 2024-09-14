/**
 * @file SubscribableSourceDetector.h
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#ifndef SUBSCRIBABLESOURCEDETECTOR_H
#define SUBSCRIBABLESOURCEDETECTOR_H

#include <sacn/cpp/source_detector.h>
#include "SubscribableNotifyHandler.h"

namespace mobilesacn::rpc {

/**
 * Wrap the SourceDetector to handle multiple subscribers.
 */
class SubscribableSourceDetector final : public SubscribableNotifyHandler,
                                         public sacn::SourceDetector::NotifyHandler
{
public:
    static SubscribableSourceDetector& get();

    SubscribableSourceDetector(const SubscribableSourceDetector&) = delete;
    SubscribableSourceDetector& operator=(const SubscribableSourceDetector&) = delete;
    ~SubscribableSourceDetector() override;

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

    explicit SubscribableSourceDetector() = default;

    static void sendSourceUpdated(const Source& source, const SendersList& senders);
    static void sendSourceExpired(const std::string& cid, const SendersList& senders);
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLESOURCEDETECTOR_H
