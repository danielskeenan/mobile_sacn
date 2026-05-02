/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "HttpServer.h"
#include "ClientSettings.h"
#include "HandlerFactory.h"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QResource>

namespace mobilesacn {

HttpServer::HttpServer(Options options, QObject *parent) :
    QObject(parent), options_(std::move(options)), wsServer_({}, QWebSocketServer::NonSecureMode)
{
    connect(&wsServer_, &QWebSocketServer::newConnection, this, &HttpServer::onWsNewConnection);
    connect(&wsServer_, &QWebSocketServer::acceptError, this, &HttpServer::onWsAcceptError);
    connect(&wsServer_, &QWebSocketServer::serverError, this, &HttpServer::onWsServerError);
}

HttpServer::~HttpServer()
{
    stop();
}

void HttpServer::run()
{
    server_.bind_to_port(options_.backend_address, kHttpPort);
    if (!wsServer_.listen(QHostAddress(QString::fromStdString(options_.backend_address)))) {
        SPDLOG_ERROR("Failed to start WebSocket server: {}", wsServer_.errorString().toStdString());
        return;
    }

    auto controller = new HttpServerController(&server_, &wsServer_, this);
    connect(
        controller, &HttpServerController::finished, controller, &HttpServerController::deleteLater);
    controller->start();
    server_.wait_until_ready();

    SPDLOG_INFO("Server listening on {}", getUrl());
}

void HttpServer::stop()
{
    if (server_.is_running()) {
        server_.stop();
        // Wait for stop.
        while (server_.is_running()) {
            QThread::msleep(100);
        }
    }

    if (wsServer_.isListening()) {
        wsServer_.close();
    }

    SPDLOG_INFO("Server stopped");
}

std::string HttpServer::getUrl()
{
    return fmt::format("http://{}:{}", options_.backend_address, kHttpPort);
}

HttpQtTaskQueue::HttpQtTaskQueue(QObject *parent) :
    QObject(parent), threadPool_(new QThreadPool(this))
{
    threadPool_->setMaxThreadCount(CPPHTTPLIB_THREAD_POOL_MAX_COUNT);
}

bool HttpQtTaskQueue::enqueue(std::function<void()> fn)
{
    threadPool_->start(std::move(fn));
    return true;
}

void HttpQtTaskQueue::shutdown()
{
    threadPool_->clear();
    threadPool_->waitForDone();
}

HttpServerController::HttpServerController(
    httplib::Server *server, QWebSocketServer *wsServer, QObject *parent) :
    QThread(parent), server_(server), wsServer_(wsServer)
{}

void HttpServerController::run()
{
    // Threading.
    server_->new_task_queue = []() { return new HttpQtTaskQueue; };

    // Logging.
    server_->set_logger([](const httplib::Request &req, const httplib::Response &res) {
        SPDLOG_DEBUG(
            "{time:%Y-%m-%d %H:%M:%S} {addr} \"{method} {path}\" {status} {size}B",
            fmt::arg("time", std::chrono::system_clock::now()),
            fmt::arg("addr", req.remote_addr),
            fmt::arg("method", req.method),
            fmt::arg("path", req.path),
            fmt::arg("status", res.status),
            fmt::arg("size", res.body.size()));
    });

    // CORS
    server_->set_post_routing_handler([](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // Settings
    server_->Get("/clientsettings", [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Cache-Control", "no-store");
        ClientSettings settings;
        auto json = settings.toJson();
        json["wsRoot"] = wsServer_->serverUrl().toString();
        res.set_content(
            QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString(), "application/json");
    });
    server_->Put("/clientsettings", [](const httplib::Request &req, httplib::Response &res) {
        if (req.get_header_value("Content-Type").starts_with("application/json")) {
            try {
                QJsonParseError err;
                const QByteArray data(req.body.data(), req.body.size());
                const auto json = QJsonDocument::fromJson(data, &err);
                if (json.isNull()) {
                    res.status = httplib::StatusCode::BadRequest_400;
                    res.set_content("Malformed request body", "text/plain");
                    return;
                }
                ClientSettings settings(json);
                settings.save();
                res.set_content("Settings saved", "text/plain");
            } catch (const std::exception &e) {
                res.status = httplib::StatusCode::BadRequest_400;
                res.set_content(e.what(), "text/plain");
            }
        } else {
            res.status = httplib::StatusCode::UnsupportedMediaType_415;
            res.set_content("Unsupported media type", "text/plain");
        }
    });

    // Catch-all
    server_->Get(R"(^/(.*)$)", &HttpServerController::serveStaticFile);

    server_->listen_after_bind();
}

void HttpServerController::serveStaticFile(const httplib::Request &req, httplib::Response &res)
{
    auto path = QString::fromStdString(req.matches[1]);

    // Match what would otherwise be a request for a directory entry.
    if (path.isEmpty() || !std::filesystem::path(path.toStdString()).has_extension()) {
        // Let client-side routing work, so serve index.html.
        path = QStringLiteral("index.html");
    }

    // Try a static file.
    const auto resourcePath = QString(":/webui/%1").arg(path);
    // This is more performant than using QFile.
    QResource resource(resourcePath);
    if (resource.isValid() && resource.data() != nullptr) {
#ifdef NDEBUG
        // Cache in production builds.
        res.set_header("Cache-Control", "max-age=3600");
#endif

        // Read contents.
        const auto bytes = resource.uncompressedData();

        // Set response mime type.
        const QMimeDatabase mimeDatabase;
        const auto mime = mimeDatabase.mimeTypeForFileNameAndData(resourcePath, bytes);

        // Send response.
        res.set_content(bytes.data(), bytes.size(), mime.name().toStdString());
        return;
    }

    // File not found.
    res.status = httplib::StatusCode::NotFound_404;
}

void HttpServer::onWsNewConnection()
{
    auto ws = wsServer_.nextPendingConnection();
    auto handler = createWebHandler(ws, this);
    const auto path = ws->requestUrl().path().toStdString();
    if (!handler) {
        SPDLOG_WARN("Unsupported websocket path: {}", ws->requestUrl().path().toStdString());
        ws->close();
        ws->deleteLater();
        return;
    }
}

void HttpServer::onWsAcceptError(QAbstractSocket::SocketError socketError)
{
    const auto errEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    SPDLOG_WARN("Error accepting new Websocket connection: {}", errEnum.valueToKey(socketError));
}

void HttpServer::onWsServerError(QWebSocketProtocol::CloseCode closeCode)
{
    SPDLOG_WARN("Websocket server error: {}", wsServer_.errorString().toStdString());
}

} // namespace mobilesacn
