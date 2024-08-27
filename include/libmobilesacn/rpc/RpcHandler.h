/**
 * @file RpcHandler.h
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#ifndef RPCHANDLER_H
#define RPCHANDLER_H

#include <QObject>
#include <span>
#include <crow/websocket.h>

namespace mobilesacn::rpc {
/**
 * Base class for RPC Handlers.
 */
class RpcHandler : public QObject
{
    Q_OBJECT

public:
    using Factory = std::function<RpcHandler*(crow::websocket::connection& ws, QObject* parent)>;
    using TextMessage = std::string_view;
    using BinaryMessage = std::span<const uint8_t>;

    explicit RpcHandler(crow::websocket::connection& ws, QObject* parent = nullptr);

    /**
     * Supported protocol name.
     */
    [[nodiscard]] virtual const char* getProtocol() = 0;

public Q_SLOTS:
    virtual void handleConnected() {}
    virtual void handleTextMessage(mobilesacn::rpc::RpcHandler::TextMessage data) {}
    virtual void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) {}
    virtual void handleClose() {}

protected:
    crow::websocket::connection& ws_;
};
} // mobilesacn::rpc

#endif //RPCHANDLER_H
