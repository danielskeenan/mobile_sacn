/**
 * @file BaseHandler.cpp
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright Apache-2.0
 */

#include "BaseHandler.h"
#include <spdlog/spdlog.h>

namespace mobilesacn {

BaseHandler::BaseHandler(QWebSocket *ws, QObject *parent) : QObject(parent), ws_(ws)
{
    ws->setParent(this);
    connect(ws, &QWebSocket::disconnected, this, &BaseHandler::onDisconnected);

    // As these slots can slow the program down, only call them when they might actually do something.
#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
    connect(ws, &QWebSocket::binaryMessageReceived, this, &BaseHandler::logBinaryMessage);
    connect(ws, &QWebSocket::textMessageReceived, this, &BaseHandler::logTextMessage);
#endif

    SPDLOG_INFO(
        "Started {} handler for client {}",
        ws->requestUrl().path().toStdString(),
        ws->peerAddress().toString().toStdString());
}

void BaseHandler::sendBinaryMessage(const QByteArrayView data) const
{
    SPDLOG_TRACE(
        "Sending binary message to {}: {} bytes",
        ws_->peerAddress().toString().toStdString(),
        data.size());
    ws_->sendBinaryMessage({data.data(), data.size()});
}

void BaseHandler::sendBinaryMessage(const uint8_t *const ptr, const qsizetype size) const
{
    // This overload exists to support flatbuffers builders.
    sendBinaryMessage({reinterpret_cast<const char *>(ptr), size});
}

void BaseHandler::sendTextMessage(const QString &str) const
{
    SPDLOG_TRACE(
        "Sending text message to {}: {} chars",
        ws_->peerAddress().toString().toStdString(),
        str.size());
    ws_->sendTextMessage(str);
}

void BaseHandler::onDisconnected()
{
    SPDLOG_INFO(
        "Closing {} handler for client {}",
        ws_->requestUrl().path().toStdString(),
        ws_->peerAddress().toString().toStdString());
    Q_EMIT(stopped(getDisplayName(), ws_->peerAddress()));
    deleteLater();
}

void BaseHandler::logBinaryMessage(const QByteArray &message)
{
    SPDLOG_TRACE(
        "Received binary message from {}: {} bytes",
        ws_->peerAddress().toString().toStdString(),
        message.size());
}

void BaseHandler::logTextMessage(const QString &message)
{
    SPDLOG_TRACE(
        "Received text message from {}: {} bytes",
        ws_->peerAddress().toString().toStdString(),
        message.size());
}

} // namespace mobilesacn
