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

class RpcHandler;

struct WsUserData
{
    std::string clientIp;
    std::string protocol;
    std::shared_ptr<RpcHandler> handler;
};

/**
 * Interface for classes that can send binary data.
 */
class WsBinarySender
{
public:
    virtual void sendBinary(const uint8_t* data, std::size_t size) = 0;
    [[nodiscard]] virtual WsUserData* getWsUserData() const = 0;
};

/**
 * Base class for RPC Handlers.
 */
class RpcHandler : public QObject, public WsBinarySender
{
    Q_OBJECT

public:
    using TextMessage = std::string_view;
    using BinaryMessage = std::span<const uint8_t>;

    explicit RpcHandler(crow::websocket::connection& ws);

    /**
     * Supported protocol name.
     */
    [[nodiscard]] virtual const char* getProtocol() const = 0;

    /**
     *
     * @return User-facing display name.
     */
    [[nodiscard]] virtual QString getDisplayName() const = 0;

    void sendBinary(const uint8_t* data, std::size_t size) override;

    [[nodiscard]] WsUserData* getWsUserData() const override
    {
        return static_cast<WsUserData*>(ws_.userdata());
    }

public Q_SLOTS:
    virtual void handleConnected() {}
    virtual void handleTextMessage(mobilesacn::rpc::RpcHandler::TextMessage data) {}
    virtual void handleBinaryMessage(mobilesacn::rpc::RpcHandler::BinaryMessage data) {}
    virtual void handleClose() {}

protected:
    std::mutex wsMutex_;
    crow::websocket::connection& ws_;
};
} // mobilesacn::rpc

#endif //RPCHANDLER_H
