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
#include "RpcHandler.h"

namespace mobilesacn::rpc {

/**
 * Base class for NotifyHandlers that need to send updates to more than one websocket.
 */
class SubscribableNotifyHandler
{
public:
    virtual ~SubscribableNotifyHandler() = default;

    void addSender(WsBinarySender* sender);
    void removeSender(WsBinarySender* sender);

protected:
    using SendersList = std::vector<WsBinarySender*>;

    std::mutex sendersMutex_;
    SendersList senders_;

    explicit SubscribableNotifyHandler() = default;

    /**
     * Called when no senders are subscribed and a new sender is added.
     *
     * @param sender The initial sender.
     * @return TRUE if the sender should be added, FALSE if not.
     */
    virtual bool startup(WsBinarySender* sender) = 0;

    /**
     * Called when removing the last sender.
     */
    virtual void shutdown() = 0;

    /**
     * Called when a new sender is added.
     */
    virtual void onSenderAdded(WsBinarySender* sender) {}

    /**
     *Send a binary message to the given senders.
     *
     * @param data
     * @param size
     */
    void sendToSenders(const uint8_t* data, std::size_t size);

    /**
     * Overload that sends data to specific senders.
     *
     * @param senders
     * @param data
     * @param size
     */
    static void sendToSenders(const SendersList& senders, const uint8_t* data, std::size_t size);

private:
    std::atomic_bool running_ = false;
};

} // mobilesacn::rpc

#endif //SUBSCRIBABLENOTIFYHANDLER_H
