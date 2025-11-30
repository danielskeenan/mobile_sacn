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
#include <boost/signals2/signal.hpp>

namespace mobilesacn::rpc {

struct SourceDetectorSource
{
    std::string cid;
    std::string name;
    std::vector<uint16_t> universes;
};

class SourceDetectorSubscriber
{
public:
    virtual void onSourceUpdated(const SourceDetectorSource& source) = 0;
    virtual void onSourceExpired(const etcpal::Uuid& cid) = 0;
};

/**
 * Wrap the SourceDetector to handle multiple subscribers.
 */
class SubscribableSourceDetector final : public SubscribableNotifyHandler<SourceDetectorSubscriber>,
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
    void connectSignals(SubscriberPtr& subscriber) override;

    bool startup(SubscriberPtr sender) override;
    void shutdown() override;

private:
    std::mutex sourcesMutex_;
    std::unordered_map<etcpal::Uuid, SourceDetectorSource> sources_;

    boost::signals2::signal<void(const SourceDetectorSource& source)> sigSourceUpdated_;
    boost::signals2::signal<void(const etcpal::Uuid& cid)> sigSourceExpired_;

    explicit SubscribableSourceDetector() = default;
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLESOURCEDETECTOR_H
