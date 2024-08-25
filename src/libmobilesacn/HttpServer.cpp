/**
 * @file HttpServer.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/HttpServer.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace mobilesacn {

HttpServer::HttpServer(Options options, QObject* parent)
    : QObject(parent),
      options_(std::move(options)) {}

void HttpServer::run()
{
    server_ = new QHttpServer(this);
    // Starting with the default port, increment the port by 1 until is can be bound.
    auto tryPort = kHttpPortStart;
    for (;;) {
        const auto port = server_->listen(
            QHostAddress(QString::fromStdString(options_.backend_address)),
            tryPort
        );
        if (port == 0) {
            ++tryPort;
        } else {
            break;
        }
    }
    spdlog::info("Server listening on {}", getUrl());
}

void HttpServer::stop()
{
    server_->deleteLater();
    server_ = nullptr;
}

std::string HttpServer::getUrl() const
{
    Q_ASSERT(server_ != nullptr && !server_->serverPorts().empty());
    return fmt::format("http://{}:{}", options_.backend_address, server_->serverPorts().front());
}

} // mobilesacn
