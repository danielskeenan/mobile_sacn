/**
 * @file HttpServer.h
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_

#include <string>
#include <QHttpServer>
#include <QWebSocketServer>
#include "rpc/RpcHandler.h"

namespace mobilesacn {
namespace detail {
class HttpServerImpl : public QAbstractHttpServer
{
    Q_OBJECT

public:
    explicit HttpServerImpl(const QString& address, QObject* parent = nullptr);

    [[nodiscard]] uint16_t getHttpPort() const
    {
        return httpPort_;
    }

protected:
    bool handleRequest(const QHttpServerRequest& request, QHttpServerResponder& responder) override;
    void missingHandler(const QHttpServerRequest& request,
                        QHttpServerResponder&& responder) override;

private:
    /** Start with this port number when finding a port to bind to. */
    static const uint16_t kHttpPortStart = 5050;

    QSet<rpc::RpcHandler*> handlers_;
    uint16_t httpPort_ = 0;

    static QDir getWebRoot();
    static QString normalizeUrlPath(const QUrl& url);
    [[nodiscard]] static bool filePathIsInWebRoot(const QString& path);
    [[nodiscard]] bool serveStaticFile(const QString& path, QHttpServerResponder& responder);

private Q_SLOTS:
    void onNewWsConnection();
};
} // detail

/**
 * HTTP web server.
 *
 * Handles serving the Web UI and RPC requests.
 */
class HttpServer : public QObject
{
    Q_OBJECT

public:
    struct Options
    {
        std::string backend_address;
        std::string sacn_address;
    };

    explicit HttpServer(Options options, QObject* parent = nullptr);

    void run();
    void stop();
    [[nodiscard]] std::string getUrl() const;

private:
    Options options_;
    detail::HttpServerImpl* server_ = nullptr;
};
} // mobilesacn

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_HTTPSERVER_H_
