/**
 * @file Application.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include "Application.h"

#include "HttpServer.h"
#include "SacnCidGenerator.h"
#include "SacnSettings.h"
#include "handler/SourceDetector.h"
#include "mobilesacn_config.h"
#include <etcpal/cpp/netint.h>
#include <sacn/cpp/common.h>
#include <spdlog/spdlog.h>

void initResources()
{
    Q_INIT_RESOURCE(webui);
}

namespace mobilesacn {

Application::Application(QObject *parent) : QObject(parent)
{
    initResources();

    // Init EtcPal.
    etcPalLogger_.SetSyslogAppName(config::kProjectName);
    if (!etcPalLogger_.Startup(etcPalLogHandler_)) {
        SPDLOG_CRITICAL(
            "Error starting the logger for the sACN subsystem.  Some logs will not be available.");
    }

    // Init sACN.
    auto result = sacn::Init(etcPalLogger_);
    if (!result.IsOk()) {
        SPDLOG_CRITICAL("Error initializing the sACN subsystem: {}", result.ToString());
    }
}

Application::~Application()
{
    // Shutdown sACN.
    sacn::Deinit();

    // Shutdown EtcPal.
    etcPalLogger_.Shutdown();
}

void Application::start(const Options &options)
{
    // Tell the CID generator about the sACN interface's MAC Address.
    auto sacnNetInterface = etcpal::netint::GetInterfaceWithIp(
        etcpal::IpAddr::FromString(options.sacn_address));
    if (!sacnNetInterface) {
        SPDLOG_CRITICAL("Could not get network interface for sACN.");
        return;
    }
    SacnCidGenerator::get().setMacAddress(sacnNetInterface->mac());

    auto sacnSettings = SacnSettings::getMutable();
    sacnSettings->sacnNetInt = *sacnNetInterface;
    sacnSettings->sacnMcastInterfaces = { SacnMcastInterface{
        .iface = {
            .ip_type = sacnNetInterface->addr().raw_type(),
            .index = sacnNetInterface->index().value(),
        },
        .status = etcpal::netint::IsUp(sacnNetInterface->index())
        ? kEtcPalErrOk
        : kEtcPalErrNotConn
    } };

    // Setup web server.
    httpServer_ = new HttpServer(
        HttpServer::Options{
            .backend_address = options.backend_address,
            .sacn_interface = *sacnNetInterface,
        },
        this);
    connect(httpServer_, &HttpServer::handlerStarted, this, &Application::handlerStarted);
    connect(httpServer_, &HttpServer::handlerStopped, this, &Application::handlerStopped);

    httpServer_->run();

    // Setup sACN Source Detector
    handler::SourceDetector::get()->startup();

    Q_EMIT(started());
}

void Application::stop()
{
    handler::SourceDetector::get()->shutdown();
    if (httpServer_) {
        httpServer_->stop();
        httpServer_->deleteLater();
        httpServer_ = nullptr;
    }
    Q_EMIT(stopped());
}

QString Application::getWebUrl() const
{
    if (httpServer_) {
        return QString::fromStdString(httpServer_->getUrl());
    }
    return {};
}

} // namespace mobilesacn
