/**
 * @file RpcHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/rpc/RpcHandler.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "libmobilesacn/rpc/ChanCheck.h"

namespace mobilesacn::rpc {

using HandlerConstructor = std::function<RpcHandler*()>;

RpcHandler* RpcHandler::getHandlerForWebsocket(std::unique_ptr<QWebSocket> ws, QObject* parent)
{
    Q_ASSERT(ws);
    static const QHash<QString, HandlerConstructor> handlers{
        { ChanCheck::kProtocol, [&ws, parent]() { return new ChanCheck(std::move(ws), parent); } },
    };

    // Skip "/ws/" in URL path.
    const auto protocol = ws->requestUrl().path().mid(4);
    const auto& ctor = handlers.value(protocol, nullptr);
    if (ctor == nullptr) {
        spdlog::error("Failed to get handler for protocol {}", protocol.toStdString());
        ws->close(QWebSocketProtocol::CloseCodeProtocolError,
                  QString("Unsupported protocol %1").arg(protocol));
        return nullptr;
    }
    return ctor();
}

const QStringList& RpcHandler::getSupportedProtocols()
{
    static const QStringList supportedProtocols{
        ChanCheck::kProtocol,
    };

    return supportedProtocols;
}

RpcHandler::RpcHandler(std::unique_ptr<QWebSocket> socket, QObject* parent)
    : QObject(parent),
      ws_(std::move(socket))
{
    ws_->setParent(this);
    connect(ws_.get(), &QWebSocket::connected, this, &RpcHandler::handleConnected,
            Qt::QueuedConnection);
    connect(ws_.get(), &QWebSocket::textMessageReceived, this, &RpcHandler::handleTextMessage,
            Qt::QueuedConnection);
    connect(ws_.get(), &QWebSocket::binaryMessageReceived, this, &RpcHandler::handleBinaryMessage,
            Qt::QueuedConnection);
    connect(ws_.get(), &QWebSocket::aboutToClose, this, &RpcHandler::handleClose,
            Qt::QueuedConnection);
    connect(ws_.get(), &QWebSocket::disconnected, this, &RpcHandler::onDisconnected,
            Qt::QueuedConnection);
}

void RpcHandler::onDisconnected()
{
    // Once the socket disconnects, no sense in keeping the handler around.
    this->deleteLater();
}

} // mobilesacn::rpc
