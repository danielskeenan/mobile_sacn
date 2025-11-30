/**
 * @file SubscribableNotifyHandler.h
 *
 * @author Dan Keenan
 * @date 9/8/24
 * @copyright GNU GPLv3
 */

#ifndef SUBSCRIBABLENOTIFYHANDLER_H
#define SUBSCRIBABLENOTIFYHANDLER_H

#include <mutex>
#include <vector>
#include <unordered_map>
#include <boost/signals2/connection.hpp>
#include <spdlog/spdlog.h>
#include "RpcHandler.h"

namespace mobilesacn::rpc {

/**
 * Base class for NotifyHandlers that need to send updates to more than one websocket.
 */
template <class SubscriberT>
class SubscribableNotifyHandler
{
protected:
    using SubscriberPtr = std::shared_ptr<SubscriberT>;

private:
    std::mutex subscriberConnectionsMutex_;

public:
    virtual ~SubscribableNotifyHandler() = default;

    void subscribe(SubscriberPtr subscriber)
    {
        std::scoped_lock subscriberConnectionsLock(subscriberConnectionsMutex_);
        if (subscriberConnections_.empty() && !running_) {
            if (!startup(subscriber)) {
                SPDLOG_CRITICAL("Failed to startup SubscribableNotifyHandler");
                return;
            }
            running_ = true;
        }
        connectSignals(subscriber);
    }

    void unsubscribe(SubscriberPtr subscriber)
    {
        std::scoped_lock subscriberConnectionsLock(subscriberConnectionsMutex_);
        subscriberConnections_.erase(subscriber);
        if (subscriberConnections_.empty() && running_) {
            shutdown();
            running_ = false;
        }
    }

protected:
    std::atomic_bool running_ = false;
    using SubscriberConnectionsMap = std::unordered_map<
        SubscriberPtr, std::vector<boost::signals2::scoped_connection> >;
    /** Map subscriber pointers to a list of their connections. */
    SubscriberConnectionsMap subscriberConnections_;

    /**
     * Called when there are no subscribers and a new subscriber is added.
     *
     * @param subscriber The initial subscriber.
     * @return TRUE if the subscriber should be added, FALSE if not.
     */
    virtual bool startup(SubscriberPtr subscriber) = 0;

    /**
     * Called when removing the last subscriber.
     */
    virtual void shutdown() = 0;

    /**
     * Setup all necessary signal connections.
     *
     * This function MUST update subscriberConnections_ with the signal connections so they can be
     * disconnected later.
     * @param subscriber
     */
    virtual void connectSignals(SubscriberPtr& subscriber) = 0;
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLENOTIFYHANDLER_H
