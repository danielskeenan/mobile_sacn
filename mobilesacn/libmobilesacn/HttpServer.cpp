/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "HttpServer.h"
#include "rpc/ChanCheck.h"
#include "rpc/ReceiveLevels.h"
#include "rpc/TransmitLevels.h"
#include <fmt/format.h>
#include <mobilesacn_config.h>
#include <set>
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QResource>

#include "ClientSettings.h"

namespace mobilesacn {

static std::set<rpc::WsUserData *> wsUserDataList;

template<class HandlerT, typename CrowT>
void setupWebsocketRoute(crow::WebSocketRule<CrowT> &rule, HttpServer *parent)
    requires(std::derived_from<HandlerT, rpc::RpcHandler>)
{
    const auto route = rule.rule();
    rule.onopen([route, parent](crow::websocket::connection &ws) {
            SPDLOG_DEBUG("Creating handler for route {}", route);
            // Deleted when socket is closed.
            const auto sacnInterface = parent->getOptions().sacn_interface;
            auto *userData = new rpc::WsUserData{
                .clientIp = ws.get_remote_ip(),
                .protocol = "",
                .handler = nullptr,
            };
            ws.userdata(userData);
            auto handler = std::shared_ptr<rpc::RpcHandler>(new HandlerT(ws));
            userData->protocol = handler->getProtocol();
            userData->handler = std::move(handler);
            wsUserDataList.insert(userData);
            userData->handler->handleConnected();
            SPDLOG_INFO("Started {} handler for client {}", userData->protocol, userData->clientIp);
        })
        .onmessage([](crow::websocket::connection &ws, const std::string &message, bool isBinary) {
            auto userData = static_cast<rpc::WsUserData *>(ws.userdata());
            if (isBinary) {
                SPDLOG_DEBUG(
                    "Received binary message from {}: {} bytes", ws.get_remote_ip(), message.size());
                userData->handler->handleBinaryMessage(
                    {reinterpret_cast<const uint8_t *>(message.data()), message.size()});
            } else {
                SPDLOG_DEBUG(
                    "Received text message from {}: {} bytes", ws.get_remote_ip(), message.size());
                userData->handler->handleTextMessage(message);
            }
        })
        .onerror([](crow::websocket::connection &ws, const std::string &message) {
            auto userData = static_cast<rpc::WsUserData *>(ws.userdata());
            SPDLOG_WARN("{} socket error: {}", userData->protocol, message);
        })
        .onclose([](crow::websocket::connection &ws,
                    const std::string &reason,
                    uint16_t with_status_code) {
            auto userData = static_cast<rpc::WsUserData *>(ws.userdata());
            // This handler gets called more than once sometimes for unknown reasons. Guard
            // against double free crashes in that case.
            if (userData->handler) {
                SPDLOG_INFO(
                    "Closing {} handler for client {}", userData->protocol, userData->clientIp);
                userData->handler->handleClose();
            } else {
                SPDLOG_DEBUG(
                    "Couldn't call {} close handler because it has been destroyed.",
                    userData->protocol);
            }
            if (wsUserDataList.erase(userData) > 0) {
                delete userData;
            } else {
                SPDLOG_DEBUG("Didn't delete userdata because it was not stored in the cache.");
            }
        });
}

HttpServer::HttpServer(Options options, QObject *parent) :
    QObject(parent), options_(std::move(options))
{
    // Setup HTTP server.
    crow::logger::setHandler(&crowLogHandler_);
    server_.bindaddr(options_.backend_address)
        .port(kHttpPort)
        .multithreaded()
        .use_compression(crow::compression::algorithm::GZIP);
    auto &cors = server_.get_middleware<crow::CORSHandler>();
    cors.global().methods(crow::HTTPMethod::Get, crow::HTTPMethod::Put).origin("*");

    // Client settings
    CROW_ROUTE(server_, "/clientsettings")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Put)([this](const crow::request &req) {
            crow::response res;
            res.set_header("Cache-Control", "no-store");
            if (req.method == crow::HTTPMethod::Get) {
                ClientSettings settings;
                auto json = settings.toJson();
                json["wsRoot"] = QString("ws://%1:%2/ws")
                                     .arg(QString::fromStdString(server_.bindaddr()))
                                     .arg(server_.port());
                res.set_header("Content-Type", "application/json; charset=utf-8");
                res.code = crow::status::OK;
                res.body = QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString();
            } else if (req.method == crow::HTTPMethod::Put) {
                res.set_header("Content-Type", "text/plain; charset=utf-8");
                if (req.get_header_value("Content-Type").starts_with("application/json")) {
                    try {
                        QJsonParseError err;
                        const QByteArray data(req.body.data(), req.body.size());
                        const auto json = QJsonDocument::fromJson(data, &err);
                        if (json.isNull()) {
                            throw std::runtime_error("Malformed request body");
                        }
                        ClientSettings settings(json);
                        settings.save();
                        res.code = crow::status::OK;
                        res.body = "Settings saved";
                    } catch (const std::exception &e) {
                        res.code = crow::status::BAD_REQUEST;
                        res.body = e.what();
                    }
                } else {
                    res.code = crow::status::UNSUPPORTED_MEDIA_TYPE;
                    res.body = "Unsupported media type";
                }
            }
            return res;
        });

    // Websocket handlers.
    setupWebsocketRoute<rpc::ChanCheck>(CROW_WEBSOCKET_ROUTE(server_, "/ws/ChanCheck"), this);
    setupWebsocketRoute<rpc::TransmitLevels>(
        CROW_WEBSOCKET_ROUTE(server_, "/ws/TransmitLevels"), this);
    setupWebsocketRoute<rpc::ReceiveLevels>(CROW_WEBSOCKET_ROUTE(server_, "/ws/ReceiveLevels"), this);

    // Static content.
    CROW_ROUTE(server_, "/").methods(crow::HTTPMethod::Get)([]() {
        return serveStaticFile("index.html");
    });
    CROW_ROUTE(server_, "/<path>").methods(crow::HTTPMethod::Get)(&HttpServer::serveStaticFile);
}

void HttpServer::run()
{
    serverHandle_ = server_.run_async();
    server_.wait_for_server_start();
    SPDLOG_INFO("Server listening on {}", getUrl());
}

void HttpServer::stop()
{
    server_.stop();
    for (const auto wsUserData : wsUserDataList) {
        delete wsUserData;
    }
    wsUserDataList.clear();
    SPDLOG_INFO("Server stopped");
}

std::string HttpServer::getUrl()
{
    return fmt::format("http://{}:{}", server_.bindaddr(), server_.port());
}

const std::filesystem::path &HttpServer::getWebRoot()
{
    // static const auto webroot = QDir(QString("%1/../%2")
    // .arg(qApp->applicationDirPath(), config::kWebPath));
    static const auto webroot = std::filesystem::canonical(
        std::filesystem::path(qApp->applicationDirPath().toStdString()) / ".." / config::kWebPath);
    return webroot;
}

crow::response HttpServer::serveStaticFile(const std::string &path)
{
    // Try a static file.
    const auto resourcePath = QString(":/webui/%1").arg(path);
    QResource resource(resourcePath);
    if (resource.isValid() && resource.data() != nullptr) {
        // Read contents
        crow::response res;
        const auto bytes = resource.uncompressedData();
        const std::string str(bytes.data(), bytes.size());
        res.write(str);

        // Set response mime type.
        const QMimeDatabase mimeDatabase;
        const auto mime = mimeDatabase.mimeTypeForFileNameAndData(resourcePath, bytes);
        res.add_header("Content-Type", mime.name().toStdString());

        return res;
    }

    // Try a directory index.
    QResource indexResource(QString("%1/index.html").arg(resourcePath));
    if (indexResource.isValid() && indexResource.data() != nullptr) {
        crow::response res;
        res.redirect(fmt::format("/{}/index.html", path));
        return res;
    }

    if (QFileInfo(resourcePath).suffix().isEmpty()) {
        // Let client-side routing work, so serve index.html.
        return serveStaticFile("index.html");
    }

    // File not found.
    crow::response res;
    res.code = 404;
    return res;
}

} // namespace mobilesacn
