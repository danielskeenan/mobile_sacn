/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/HttpServer.h"
#include <mobilesacn_config.h>
#include <QCoreApplication>
#include <QDir>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <QMimeDatabase>

namespace mobilesacn {

template <typename T>
uint16_t startWithPort(const QString& address, uint16_t startPort, T* server)
{
    // Starting with the default port, increment the port by 1 until is can be bound.
    auto tryPort = startPort;
    for (;;) {
        const auto port = server->listen(QHostAddress(address), tryPort);
        if (port == 0) {
            ++tryPort;
        } else {
            break;
        }
    }
    return tryPort;
}

detail::HttpServerImpl::HttpServerImpl(const QString& address, QObject* parent)
    : QAbstractHttpServer(parent)
{
    // HTTP Server
    httpPort_ = startWithPort(address, kHttpPortStart, this);
    connect(this, &HttpServerImpl::newWebSocketConnection, this,
            &HttpServerImpl::onNewWsConnection);
    spdlog::debug("HTTP server listening on {}:{}", address.toStdString(), httpPort_);
}

bool detail::HttpServerImpl::handleRequest(const QHttpServerRequest& request,
                                           QHttpServerResponder& responder)
{
    if (request.method() == QHttpServerRequest::Method::Get) {
        spdlog::debug("GET: {}", request.url().toString().toStdString());
        const auto urlPath = normalizeUrlPath(request.url());

        // Websocket path
        if (urlPath.startsWith("ws/")) {
            // Request needs to be marked as handled in order for QAbstractHttpServer to notice that
            // it's a websocket upgrade request.
            return true;
        }
        // Ask for Websocket URL.
        if (urlPath == "ws_url") {
            responder.write(
                // wsServer_->serverUrl().toString(QUrl::FullyEncoded).toUtf8(),
                QString("ws://%1:%2").arg(request.url().host(QUrl::FullyEncoded)).arg(httpPort_).
                toUtf8(),
                {
                    { "Content-Type", "text/plain" },
                    { "Access-Control-Allow-Origin", "*" }
                }
            );
            return true;
        }

        // Static content
        const auto filePath = QFileInfo(getWebRoot().filePath(urlPath));
        // Only serve files inside the webroot. Otherwise, pretend they don't exist.
        if (!filePathIsInWebRoot(filePath.filePath())) {
            return false;
        }

        // Request for real file.
        if (!filePath.suffix().isEmpty()) {
            if (filePath.isFile()) {
                return serveStaticFile(filePath.filePath(), responder);
            }
            return false;
        }
        // Directory index (also serves root path).
        if (filePath.isDir()) {
            const auto indexPath = QFileInfo(QDir(filePath.filePath()).filePath("index.html"));
            if (indexPath.isFile()) {
                return serveStaticFile(indexPath.filePath(), responder);
            }
            return false;
        }

        // Otherwise, serve index.html so client-side routing can do its thing.
        const auto indexPath = getWebRoot().filePath("index.html");
        return serveStaticFile(indexPath, responder);
    }

    responder.write(QHttpServerResponder::StatusCode::MethodNotAllowed);
    return true;
}

void detail::HttpServerImpl::missingHandler(const QHttpServerRequest& request,
                                            QHttpServerResponder&& responder)
{
    responder.write(QHttpServerResponder::StatusCode::NotFound);
}

QDir detail::HttpServerImpl::getWebRoot()
{
    static const auto webroot = QDir(QString("%1/../%2")
        .arg(qApp->applicationDirPath(), config::kWebPath));
    return webroot;
}

QString detail::HttpServerImpl::normalizeUrlPath(const QUrl& url)
{
    // Remove leading "/" from URL path, otherwise filesystem functions will try to treat it
    // as absolute.
    auto path = url.path().mid(1);
    while (path.endsWith("/")) {
        path.chop(1);
    }
    return path;
}

bool detail::HttpServerImpl::
serveStaticFile(const QString& path, QHttpServerResponder& responder)
{
    spdlog::debug("Serving static file: {}", path.toStdString());
    const auto mimeType = QMimeDatabase().mimeTypeForFile(path);
    auto staticFile = new QFile(path, this);
    if (!staticFile->open(QIODevice::ReadOnly)) {
        responder.write(QHttpServerResponder::StatusCode::InternalServerError);
        staticFile->deleteLater();
        return true;
    }
    // Per doc, responder takes ownership of file, then delete it when done.
    responder.write(
        staticFile,
        {
            { "Content-Type", mimeType.name().toUtf8() },
        }
    );
    return true;
}

bool detail::HttpServerImpl::filePathIsInWebRoot(const QString& path)
{
    static const auto webroot = getWebRoot().canonicalPath();
    const auto canonical = QFileInfo(path).canonicalFilePath();
    const auto mismatch = std::mismatch(
        webroot.cbegin(), webroot.cend(),
        canonical.cbegin(), canonical.cend()
    );
    return mismatch.first == webroot.cend();
}

void detail::HttpServerImpl::onNewWsConnection()
{
    auto ws = nextPendingWebSocketConnection();
    if (!ws) {
        return;
    }
    const auto clientAddress = ws->peerAddress().toString().toStdString();
    // Skip "/ws/" in URL path.
    const auto protocol = ws->requestUrl().path().mid(4);
    spdlog::debug("Requested handler for protocol {}", protocol.toStdString());
    auto handler = rpc::RpcHandler::getHandlerForWebsocket(std::move(ws), this);
    if (handler == nullptr) {
        return;
    }

    handlers_.insert(handler);
    // Remove handler pointer from set when it's destroyed.
    connect(handler, &rpc::RpcHandler::destroyed, [this, handler, clientAddress, protocol]() {
        // handler is a dead pointer at this point! Don't ask it for anything!
        handlers_.remove(handler);
        spdlog::info("Closed {} handler for client {}", protocol.toStdString(), clientAddress);
    });
    spdlog::info("Started {} handler for client {}", protocol.toStdString(), clientAddress);
}

HttpServer::HttpServer(Options options, QObject* parent)
    : QObject(parent),
      options_(std::move(options)) {}

void HttpServer::run()
{
    server_ = new detail::HttpServerImpl(QString::fromStdString(options_.backend_address), this);
    spdlog::info("Server listening on {}", getUrl());
}

void HttpServer::stop()
{
    server_->deleteLater();
    server_ = nullptr;
    spdlog::info("Server stopped");
}

std::string HttpServer::getUrl() const
{
    Q_ASSERT(server_ != nullptr);
    return fmt::format("http://{}:{}", options_.backend_address, server_->getHttpPort());
}

} // mobilesacn
