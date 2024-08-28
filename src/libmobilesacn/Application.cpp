/**
 * @file Application.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/Application.h"

#include <etcpal/cpp/netint.h>
#include <spdlog/spdlog.h>
#include <sacn/cpp/common.h>
#include "mobilesacn_config.h"
#include "libmobilesacn/HttpServer.h"
#include "libmobilesacn/SacnCidGenerator.h"

namespace mobilesacn {

Application::Application(QObject* parent)
    : QObject(parent)
{
    // Init EtcPal.
    etcPalLogger_.SetSyslogAppName(config::kProjectName);
    if (!etcPalLogger_.Startup(etcPalLogHandler_)) {
        spdlog::critical(
            "Error starting the logger for the sACN subsystem.  Some logs will not be available.");
    }

    // Init sACN.
    auto result = sacn::Init(etcPalLogger_);
    if (!result.IsOk()) {
        spdlog::critical("Error initializing the sACN subsystem: {}", result.ToString());
    }
}

Application::~Application()
{
    // Shutdown sACN.
    sacn::Deinit();

    // Shutdown EtcPal.
    etcPalLogger_.Shutdown();
}

void Application::run(const Options& options)
{
    // Tell the CID generator about the sACN interface's MAC Address.
    auto sacnNetInterface = etcpal::netint::GetInterfaceWithIp(
        etcpal::IpAddr::FromString(options.sacn_address));
    if (!sacnNetInterface) {
        spdlog::critical("Could not get network interface for sACN.");
        return;
    }
    SacnCidGenerator::get().setMacAddress(sacnNetInterface->mac());
    // Setup web server.
    httpServer_ = new HttpServer(
        HttpServer::Options{
            .backend_address = options.backend_address,
            .sacn_interface = *sacnNetInterface,
        },
        this
    );

    httpServer_->run();
}

void Application::stop()
{
    if (httpServer_) {
        httpServer_->stop();
        httpServer_->deleteLater();
        httpServer_ = nullptr;
    }
}

std::string Application::getWebUrl() const
{
    if (httpServer_) {
        return httpServer_->getUrl();
    }
    return "";
}

} // mobilesacn
