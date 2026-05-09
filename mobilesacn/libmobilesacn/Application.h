/**
 * @file Application.h
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_APPLICATION_H
#define MOBILE_SACN_APPLICATION_H

#define CROW_DISABLE_STATIC_DIR

#include "EtcPalLogHandler.h"
#include <filesystem>
#include <memory>
#include <string>
#include <QHostAddress>
#include <QObject>

namespace mobilesacn {
class HttpServer;

/**
 * Application instance.
 */
class Application : public QObject
{
    Q_OBJECT
public:
    struct Options
    {
        std::string backend_address;
        std::string sacn_address;
    };

    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    void start(const Options &options);
    void stop();
    bool isRunning() const { return httpServer_ != nullptr; }
    [[nodiscard]] QString getWebUrl() const;

Q_SIGNALS:
    void started();
    void stopped();
    void handlerStarted(const QString &displayName, const QHostAddress &clientAddress);
    void handlerStopped(const QString &displayName, const QHostAddress &clientAddress);

private:
    etcpal::Logger etcPalLogger_;
    EtcPalLogHandler etcPalLogHandler_;
    HttpServer *httpServer_ = nullptr;
};
} // namespace mobilesacn

#endif //MOBILE_SACN_APPLICATION_H
