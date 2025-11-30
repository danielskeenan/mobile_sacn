/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "HttpServer.h"
#include <mobilesacn_config.h>
#include <QCoreApplication>
#include <QDir>
#include <set>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "rpc/ChanCheck.h"
#include "rpc/ReceiveLevels.h"
#include "rpc/TransmitLevels.h"

namespace mobilesacn {

static std::set<rpc::WsUserData*> wsUserDataList;

template <class HandlerT, typename CrowT>
void setupWebsocketRoute(crow::WebSocketRule<CrowT>& rule, HttpServer* parent)
    requires(std::derived_from<HandlerT, rpc::RpcHandler>)
{
    const auto route = rule.rule();
    rule
            .onopen([route, parent](crow::websocket::connection& ws) {
                SPDLOG_DEBUG("Creating handler for route {}", route);
                // Deleted when socket is closed.
                const auto sacnInterface = parent->getOptions().sacn_interface;
                auto* userData = new rpc::WsUserData{
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
                SPDLOG_INFO("Started {} handler for client {}", userData->protocol,
                             userData->clientIp);
            })
            .onmessage([](crow::websocket::connection& ws, const std::string& message,
                          bool isBinary) {
                auto userData = static_cast<rpc::WsUserData*>(ws.userdata());
                if (isBinary) {
                    SPDLOG_DEBUG("Received binary message from {}: {} bytes", ws.get_remote_ip(),
                                  message.size());
                    userData->handler->handleBinaryMessage({
                        reinterpret_cast<const uint8_t*>(message.data()),
                        message.size()
                    });
                } else {
                    // Handle pings.
                    if (message == "ping") {
                        ws.send_text("pong");
                        return;
                    }
                    SPDLOG_DEBUG("Received text message from {}: {} bytes", ws.get_remote_ip(),
                                  message.size());
                    userData->handler->handleTextMessage(message);
                }
            })
            .onerror([](crow::websocket::connection& ws, const std::string& message) {
                auto userData = static_cast<rpc::WsUserData*>(ws.userdata());
                SPDLOG_WARN("{} socket error: {}", userData->protocol, message);
            })
            .onclose([](crow::websocket::connection& ws, const std::string& reason) {
                auto userData = static_cast<rpc::WsUserData*>(ws.userdata());
                SPDLOG_INFO("Closing {} handler for client {}", userData->protocol,
                             userData->clientIp);
                // This handler gets called more than once sometimes for unknown reasons. Guard
                // against double free crashes in that case.
                if (userData->handler) {
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

HttpServer::HttpServer(Options options, QObject* parent)
    : QObject(parent),
      options_(std::move(options))
{
    // Set working directory. This is needed for static file serving to work correctly.
    std::filesystem::current_path(getWebRoot());
    // Setup HTTP server.
    crow::logger::setHandler(&crowLogHandler_);
    server_.bindaddr(options_.backend_address)
            .port(kHttpPort)
            .multithreaded()
            .use_compression(crow::compression::algorithm::GZIP);
    auto& cors = server_.get_middleware<crow::CORSHandler>();
    cors.global()
            .methods(crow::HTTPMethod::Get)
            .origin("*");

    // Websocket url.
    CROW_ROUTE(server_, "/ws_url").methods(crow::HTTPMethod::Get)
    ([this](crow::response& res) {
        const auto url = fmt::format("ws://{}:{}/ws", server_.bindaddr(), server_.port());
        res.set_header("Content-Type", "text/plain");
        res.end(url);
    });

    // Websocket handlers.
    setupWebsocketRoute<rpc::ChanCheck>(
        CROW_WEBSOCKET_ROUTE(server_, "/ws/ChanCheck"), this);
    setupWebsocketRoute<rpc::TransmitLevels>(
        CROW_WEBSOCKET_ROUTE(server_, "/ws/TransmitLevels"), this);
    setupWebsocketRoute<rpc::ReceiveLevels>(
        CROW_WEBSOCKET_ROUTE(server_, "/ws/ReceiveLevels"), this);

    // Static content.
    CROW_ROUTE(server_, "/").methods(crow::HTTPMethod::Get)
    ([](crow::response& res) {
        res.set_static_file_info_unsafe("index.html");
        res.end();
    });
    CROW_ROUTE(server_, "/<path>").methods(crow::HTTPMethod::Get)
    ([this](crow::response& res, const std::string& path) {
        // Crow requires things it serves to be relative to current working directory
        // (this is why cwd in constructor).
        auto cleanPath = path;
        crow::utility::sanitize_filename(cleanPath);
        // Directory index.
        const auto absPath = std::filesystem::current_path() / cleanPath;
        if (std::filesystem::is_directory(absPath)) {
            res.redirect(fmt::format("/{}/index.html", path));
        } else {
            res.set_static_file_info_unsafe(cleanPath);
            if (res.code == 404) {
                // Serve index.html so client-side routing can work.
                res.set_static_file_info_unsafe("index.html");
            }
        }

        res.end();
    });
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

const std::filesystem::path& HttpServer::getWebRoot()
{
    // static const auto webroot = QDir(QString("%1/../%2")
    // .arg(qApp->applicationDirPath(), config::kWebPath));
    static const auto webroot = std::filesystem::canonical(
        std::filesystem::path(qApp->applicationDirPath().toStdString()) / ".." / config::kWebPath);
    return webroot;
}

} // mobilesacn
