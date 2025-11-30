/**
 * @file Caffeine_Windows.cpp
 *
 * @author Dan Keenan
 * @date 11/30/2025
 * @copyright GPL-3.0
 */

#include "Caffeine.h"
#include <windows.h>
#include <spdlog/spdlog.h>

namespace mobilesacn {

void Caffeine::setActive(const bool active)
{
    if (active) {
        SPDLOG_INFO("Suppressing sleep");
        if (SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS) == NULL) {
            SPDLOG_WARN("Failed to suppress sleep");
        }
    } else {
        SPDLOG_INFO("Unsuppressing sleep");
        if (SetThreadExecutionState(ES_CONTINUOUS) == NULL) {
            SPDLOG_WARN("Failed to unsuppress sleep");
        }
    }

    active_ = active;
}

} // namespace mobilesacn
