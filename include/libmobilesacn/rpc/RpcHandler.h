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
#include <etcpal/cpp/inet.h>
#include <sacn/common.h>

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

    struct WsUserData
    {
        etcpal::NetintInfo sacnNetInt;
        std::string clientIp;
        std::string protocol;
        RpcHandler* handler;
    };

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

    [[nodiscard]] const WsUserData* getWsUserData() const
    {
        return static_cast<WsUserData*>(ws_.userdata());
    }

    [[nodiscard]] std::vector<SacnMcastInterface>& getSacnMcastInterfaces()
    {
        return sacnMcastInterfaces_;
    }

private:
    std::vector<SacnMcastInterface> sacnMcastInterfaces_;
};
} // mobilesacn::rpc

#endif //RPCHANDLER_H
