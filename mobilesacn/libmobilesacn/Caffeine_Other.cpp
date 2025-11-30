/**
 * @file Caffeine_Other.cpp
 *
 * @author Dan Keenan
 * @date 11/30/25
 * @copyright GPL-3.0
 */

#include "Caffeine.h"
#include <spdlog/spdlog.h>

namespace mobilesacn {
void Caffeine::setActive(const bool active)
{
    SPDLOG_WARN("Suppressing sleep is not available on this platform.");
    active_ = active;
}
} // namespace mobilesacn
