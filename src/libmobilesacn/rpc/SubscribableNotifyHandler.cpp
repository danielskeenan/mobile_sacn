/**
 * @file SubscribableNotifyHandler.cpp
 *
 * @author Dan Keenan
 * @date 9/8/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/SubscribableNotifyHandler.h>
#include <spdlog/spdlog.h>

namespace mobilesacn::rpc {

void SubscribableNotifyHandler::addSender(WsBinarySender* sender)
{
    std::scoped_lock sendersLock(sendersMutex_);
    if (senders_.empty() && !running_) {
        if (!startup(sender)) {
            spdlog::critical("Failed to startup SubscribableNotifyHandler");
            return;
        } else {
            running_ = true;
        }
    }
    senders_.push_back(sender);
    onSenderAdded(sender);
}

void SubscribableNotifyHandler::removeSender(WsBinarySender* sender)
{
    std::scoped_lock sendersLock(sendersMutex_);
    std::erase(senders_, sender);
    if (senders_.empty() && running_) {
        shutdown();
        running_ = false;
    }
}

void SubscribableNotifyHandler::sendToSenders(const SendersList& senders, const uint8_t* data,
                                              std::size_t size)
{
    for (auto sender : senders) {
        sender->sendBinary(data, size);
    }
}

void SubscribableNotifyHandler::sendToSenders(const uint8_t* data, std::size_t size)
{
    std::lock_guard lock(sendersMutex_);
    sendToSenders(senders_, data, size);
}

} // mobilesacn::rpc
