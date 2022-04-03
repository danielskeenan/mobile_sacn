/**
 * @file Application.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/Application.h"
#include <spdlog/spdlog.h>
#include "mobilesacn_config.h"
#include <algorithm>
#include "libmobilesacn/HttpServer.h"
#include "etcpal_netint/NetIntInfo.h"

namespace mobilesacn {

static const auto kEtcPalFeatures = ETCPAL_FEATURE_LOGGING | ETCPAL_FEATURE_NETINTS;

Application::Application(Options options) :
    options_(std::move(options)) {
  // Init EtcPal.
  etcpal_init(kEtcPalFeatures);
  etc_pal_logger_.SetSyslogAppName(config::kProjectName);
  if (!etc_pal_logger_.Startup(etc_pal_log_handler_)) {
    spdlog::critical("Error starting the logger for the sACN subsystem.  Some logs will not be available.");
  }

  if (!options_.sacn_address.IsValid()) {
    options_.sacn_address = etcpal_netint::GetDefaultInterface(etcpal::IpAddrType::kV4)->GetAddr();
  }

  // Init sACN.
  auto result = sacn::Init(etc_pal_logger_);
  if (!result.IsOk()) {
    spdlog::critical("Error initializing the sACN subsystem: {}", result.ToCString());
  }

  // Setup web server.
  http_server_.reset(new HttpServer(
      {
          .backend_address = options_.backend_address,
          .backend_port = options_.backend_port,
          .sacn_address = options_.sacn_address,
      }
  ));
}

Application::~Application() {
  // Shutdown sACN.
  sacn::Deinit();

  // Shutdown EtcPal.
  etc_pal_logger_.Shutdown();
  etcpal_deinit(kEtcPalFeatures);
}

int Application::Run() {
  http_server_->Run();

  return 0;
}

} // mobilesacn
