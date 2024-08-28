/**
 * @file CrowLogHandler.cpp
 *
 * @author Dan Keenan
 * @date 8/27/24
 * @copyright GNU GPLv3
 */

#include <libmobilesacn/CrowLogHandler.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace mobilesacn {

void CrowLogHandler::log(std::string message, crow::LogLevel crowLogLevel)
{
    // Convert Crow's log levels to spdlog's.
    const spdlog::level::level_enum level = [crowLogLevel]() {
        switch (crowLogLevel) {
        case crow::LogLevel::Debug:
        case crow::LogLevel::Info:
            // Crow uses Info more often than it should...
            return spdlog::level::level_enum::debug;
        case crow::LogLevel::Warning:
            return spdlog::level::level_enum::warn;
        case crow::LogLevel::Error:
            return spdlog::level::level_enum::err;
        case crow::LogLevel::Critical:
            return spdlog::level::level_enum::critical;
        }
        // Unhandled log levels are an error.
        return spdlog::level::err;
    }();

    // Do the thing.
    spdlog::log(level, message);
}

} // mobilesacn
