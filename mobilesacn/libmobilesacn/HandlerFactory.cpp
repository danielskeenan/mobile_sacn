/**
 * @file HandlerFactory.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#include "HandlerFactory.h"
#include "handler/ChanCheck.h"
#include "handler/TransmitLevels.h"

namespace mobilesacn {

template<class Handler>
struct HandlerFactory
{
    Handler *operator()(QWebSocket *ws, QObject *parent) { return new Handler(ws, parent); }
};

// Add handlers here!
static const QHash<QString, std::function<BaseHandler *(QWebSocket *, QObject *)>> HANDLERS{
    {"/ChanCheck", HandlerFactory<handler::ChanCheck>{}},
    {"/TransmitLevels", HandlerFactory<handler::TransmitLevels>{}},
    // {"/ReceiveLevels", HandlerFactory<handler::ReceiveLevels>{}},
};

BaseHandler *createWebHandler(QWebSocket *ws, QObject *parent)
{
    const auto factory = HANDLERS.find(ws->requestUrl().path());
    if (factory == HANDLERS.cend()) {
        return nullptr;
    }
    return (*factory)(ws, parent);
}

} // namespace mobilesacn
