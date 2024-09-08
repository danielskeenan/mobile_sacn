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
    const auto wsUserData = sender->getWsUserData();
    if (senders_.empty()) {
        startup(sender);
    }
    senders_.emplace_back(sender);
}

void SubscribableNotifyHandler::removeSender(WsBinarySender* sender)
{
    std::scoped_lock sendersLock(sendersMutex_);
    std::erase(senders_, sender);
    if (senders_.empty()) {
        shutdown();
    }
}

void SubscribableNotifyHandler::sendToSenders(const SendersList& senders, const uint8_t* data,
                                              std::size_t size)
{
    for (auto sender : senders) {
        sender->sendBinary(data, size);
    }
}

long EtcPalMcastNetintIdComparator(EtcPalMcastNetintId a, EtcPalMcastNetintId b)
{
    return a.index - b.index;
}

} // mobilesacn::rpc
