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

#include "libmobilesacn/rpc/ChanCheck.h"

namespace mobilesacn {

static std::set<rpc::RpcHandler::WsUserData*> wsUserDataList;

template <class HandlerT, typename CrowT>
void setupWebsocketRoute(crow::WebSocketRule<CrowT>& rule, HttpServer* parent)
    requires(std::derived_from<HandlerT, rpc::RpcHandler>)
{
    const auto route = rule.rule();
    rule
            .onopen([route, parent](crow::websocket::connection& ws) {
                spdlog::debug("Creating handler for route {}", route);
                // Deleted when socket is closed.
                auto* userData = new rpc::RpcHandler::WsUserData{
                    .sacnNetInt = parent->getOptions().sacn_interface,
                    .clientIp = ws.get_remote_ip(),
                    .protocol = "",
                    .handler = nullptr,
                };
                ws.userdata(userData);
                auto handler = std::unique_ptr<rpc::RpcHandler>(new HandlerT(ws, nullptr));
                userData->protocol = handler->getProtocol();
                userData->handler = std::move(handler);
                wsUserDataList.insert(userData);
                spdlog::info("Started {} handler for client {}", userData->protocol,
                             userData->clientIp);
            })
            .onmessage([](crow::websocket::connection& ws, const std::string& message,
                          bool isBinary) {
                auto userData = static_cast<rpc::RpcHandler::WsUserData*>(ws.userdata());
                if (isBinary) {
                    spdlog::debug("Received binary message from {}: {} bytes", ws.get_remote_ip(),
                                  message.size());
                    userData->handler->handleBinaryMessage({
                        reinterpret_cast<const uint8_t*>(message.data()),
                        message.size()
                    });
                } else {
                    spdlog::debug("Received text message from {}: {} bytes", ws.get_remote_ip(),
                                  message.size());
                    userData->handler->handleTextMessage(message);
                }
            })
            .onerror([](crow::websocket::connection& ws, const std::string& message) {
                auto userData = static_cast<rpc::RpcHandler::WsUserData*>(ws.userdata());
                spdlog::warn("{} socket error: {}", userData->protocol, message);
            })
            .onclose([](crow::websocket::connection& ws, const std::string& reason) {
                auto userData = static_cast<rpc::RpcHandler::WsUserData*>(ws.userdata());
                spdlog::info("Closing {} handler for client {}", userData->protocol,
                             userData->clientIp);
                wsUserDataList.erase(userData);
                delete userData;
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
    setupWebsocketRoute<rpc::ChanCheck>(CROW_WEBSOCKET_ROUTE(server_, "/ws/ChanCheck"), this);

    // Static content.
    CROW_ROUTE(server_, "/").methods(crow::HTTPMethod::Get)
    ([](crow::response& res) {
        res.set_static_file_info_unsafe("index.html");
        res.end();
    });
    CROW_ROUTE(server_, "/<path>").methods(crow::HTTPMethod::Get)
    ([this](crow::response& res, const std::string& path) {
        // Relative to current working directory (this is why cwd in constructor).
        res.set_static_file_info(path);
        if (res.code == 404) {
            // Serve index.html so client-side routing can work.
            res.set_static_file_info_unsafe("index.html");
        }
        res.end();
    });
}

void HttpServer::run()
{
    serverHandle_ = server_.run_async();
    server_.wait_for_server_start();
    spdlog::info("Server listening on {}", getUrl());
}

void HttpServer::stop()
{
    server_.stop();
    for (const auto wsUserData : wsUserDataList) {
        delete wsUserData;
    }
    wsUserDataList.clear();
    spdlog::info("Server stopped");
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
