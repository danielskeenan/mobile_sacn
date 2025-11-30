/**
 * @file EtcPalLogHandler.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include "EtcPalLogHandler.h"
#include <spdlog/spdlog.h>

namespace mobilesacn {

void EtcPalLogHandler::HandleLogMessage(const EtcPalLogStrings &strings) {
  // Ensure there is something to log.
  if (strings.raw == nullptr) {
    SPDLOG_ERROR("!!!UNDEFINED LOG MESSAGE!!!");
    return;
  }

  // Convert ETC's log levels to spdlog's.
  const spdlog::level::level_enum level = [&strings]() {
    switch (strings.priority) {
      case ETCPAL_LOG_EMERG:
      case ETCPAL_LOG_ALERT:
      case ETCPAL_LOG_CRIT:return spdlog::level::critical;
      case ETCPAL_LOG_ERR:return spdlog::level::err;
      case ETCPAL_LOG_WARNING: return spdlog::level::warn;
      case ETCPAL_LOG_NOTICE:
      case ETCPAL_LOG_INFO:return spdlog::level::info;
      case ETCPAL_LOG_DEBUG:return spdlog::level::debug;
    }
    // Unhandled log levels are an error.
    return spdlog::level::err;
  }();

  // Do the thing.
  spdlog::log(level, strings.raw);
}

} // mobilesacn
