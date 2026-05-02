/**
 * @file BaseHandler.h
 *
 * @author Dan Keenan
 * @date 5/2/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_HANDLER_BASEHANDLER_H
#define MOBILESACN_LIBMOBILESACN_HANDLER_BASEHANDLER_H

#include <QWebSocket>

namespace mobilesacn {

/**
 * Base class for web handlers.
 *
 * When constructed, the websocket is already connected. When the websocket is disconnected, the handler deletes itself.
 *
 * The handler takes ownership of the websocket.
 */
class BaseHandler : public QObject
{
    Q_OBJECT

public:
    explicit BaseHandler(QWebSocket *ws, QObject *parent = nullptr);

    /**
     * Supported protocol name.
     */
    [[nodiscard]] virtual const char *getProtocol() const = 0;

    /**
     * User-facing display name.
     */
    [[nodiscard]] virtual QString getDisplayName() const = 0;

protected:
    [[nodiscard]] QWebSocket *ws() const { return ws_; }
    void sendBinaryMessage(QByteArrayView data) const;
    void sendBinaryMessage(const uint8_t *ptr, qsizetype size) const;
    void sendTextMessage(const QString &str) const;

private:
    QWebSocket *ws_;

private Q_SLOTS:
    void logBinaryMessage(const QByteArray &message);
    void logTextMessage(const QString &message);
    void onDisconnected();
};

} // namespace mobilesacn

#endif //MOBILESACN_LIBMOBILESACN_HANDLER_BASEHANDLER_H
