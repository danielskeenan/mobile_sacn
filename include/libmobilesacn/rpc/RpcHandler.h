/**
 * @file RpcHandler.h
 *
 * @author Dan Keenan
 * @date 8/26/24
 * @copyright GNU GPLv3
 */

#ifndef RPCHANDLER_H
#define RPCHANDLER_H

#include <QWebSocket>

namespace mobilesacn::rpc {
/**
 * Base class for RPC Handlers.
 */
class RpcHandler : public QObject
{
    Q_OBJECT

public:
    [[nodiscard]] static const QStringList& getSupportedProtocols();
    [[nodiscard]] static RpcHandler* getHandlerForWebsocket(
        std::unique_ptr<QWebSocket> ws, QObject* parent);

    /**
     *
     * @param socket This handler takes ownership of the passed socket.
     */
    explicit RpcHandler(std::unique_ptr<QWebSocket> socket, QObject* parent = nullptr);

    /**
     * Supported Websocket subprotocol.
     */
    [[nodiscard]] virtual const char* getProtocol() = 0;

protected:
    std::unique_ptr<QWebSocket> ws_;

protected Q_SLOTS:
    virtual void handleConnected() {}
    virtual void handleTextMessage(const QString& data) {}
    virtual void handleBinaryMessage(const QByteArray& data) {}
    virtual void handleClose() {}

private Q_SLOTS:
    void onDisconnected();
};
} // mobilesacn::rpc

#endif //RPCHANDLER_H
