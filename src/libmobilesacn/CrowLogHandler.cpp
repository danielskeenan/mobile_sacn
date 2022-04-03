/**
 * @file CrowLogHandler.cpp
 *
 * @author Dan Keenan
 * @date 4/3/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/CrowLogHandler.h"
#include <spdlog/spdlog.h>

namespace mobilesacn {

void CrowLogHandler::log(std::string message, crow::LogLevel level) {
  // Convert Crow's log levels to spdlog's.
  const spdlog::level::level_enum spdlog_level = [level]() {
    switch (level) {
      case crow::LogLevel::Critical:return spdlog::level::critical;
      case crow::LogLevel::Error:return spdlog::level::err;
      case crow::LogLevel::Warning: return spdlog::level::warn;
      case crow::LogLevel::Info:return spdlog::level::info;
      case crow::LogLevel::Debug:return spdlog::level::debug;
    }
    // Unhandled log levels are an error.
    return spdlog::level::err;
  }();

  spdlog::log(spdlog_level, message);
}

} // mobilesacn
