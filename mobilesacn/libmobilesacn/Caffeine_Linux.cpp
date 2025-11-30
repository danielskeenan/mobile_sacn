/**
 * @file Caffeine_Linux.cpp
 *
 * @author Dan Keenan
 * @date 11/30/25
 * @copyright GPL-3.0
 */

#include "Caffeine.h"
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <QApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>

namespace mobilesacn {

void Caffeine::setActive(bool active)
{
    if (active) {
        SPDLOG_INFO("Suppressing sleep");
        QDBusInterface fdLoginManager(
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            QDBusConnection::systemBus());
        const auto inhibitHandle = fdLoginManager.call(
            "Inhibit",
            "sleep:idle",
            qApp->applicationDisplayName(),
            qApp->translate("Caffeine", "Application is handling clients"),
            "block");
        if (fdLoginManager.lastError().isValid()) {
            const auto err = fdLoginManager.lastError();
            SPDLOG_ERROR(
                "Could not inhibit: {} {}", err.name().toStdString(), err.message().toStdString());
            return;
        }
        const auto fd = inhibitHandle.arguments().first().value<QDBusUnixFileDescriptor>();
        inhibitHandle_ = dup(fd.fileDescriptor());

    } else {
        if (inhibitHandle_ != -1) {
            SPDLOG_INFO("Unsuppressing sleep");
            if (!close(inhibitHandle_)) {
                SPDLOG_WARN("Could not release sleep inhibit handle: {}", std::strerror(errno));
            }
        }
    }

    active_ = active;
}

} // namespace mobilesacn
