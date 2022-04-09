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
#include "libmobilesacn/HttpServer.h"
#include "etcpal_netint/NetIntInfo.h"

namespace mobilesacn {

Application::Application() {
  // Init EtcPal.
  etc_pal_logger_.SetSyslogAppName(config::kProjectName);
  if (!etc_pal_logger_.Startup(etc_pal_log_handler_)) {
    spdlog::critical("Error starting the logger for the sACN subsystem.  Some logs will not be available.");
  }

  // Init sACN.
  auto result = sacn::Init(etc_pal_logger_);
  if (!result.IsOk()) {
    spdlog::critical("Error initializing the sACN subsystem: {}", result.ToCString());
  }
}

Application::~Application() {
  // Shutdown sACN.
  sacn::Deinit();

  // Shutdown EtcPal.
  etc_pal_logger_.Shutdown();
}

void Application::Run(Options options) {
  if (!options.sacn_address.IsValid()) {
    options.sacn_address = etcpal_netint::GetDefaultInterface(etcpal::IpAddrType::kV4)->GetAddr();
  }

  // Setup web server.
  http_server_ = std::make_unique<HttpServer>(
      HttpServer::Options{
          .backend_address = options.backend_address,
          .sacn_address = options.sacn_address,
      }
  );

  http_server_->Run();
}

void Application::Stop() {
  if (http_server_) {
    http_server_->Stop();
    http_server_.reset();
  }
}

std::string Application::GetWebUrl() const {
  if (http_server_) {
    return http_server_->GetUrl();
  }
  return "";
}

} // mobilesacn
