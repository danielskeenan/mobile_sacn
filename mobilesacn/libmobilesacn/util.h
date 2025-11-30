/**
 * @file util.h
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#ifndef UTIL_H
#define UTIL_H

#include <chrono>

namespace mobilesacn {

inline uint64_t getNowInMilliseconds()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

} // mobilesacn

#endif //UTIL_H
