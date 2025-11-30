/**
 * @file Caffeine.cpp
 *
 * @author Dan Keenan
 * @date 11/30/25
 * @copyright GPL-3.0
 */

#include "Caffeine.h"

namespace mobilesacn {

Caffeine *Caffeine::get()
{
    static Caffeine *instance = nullptr;
    if (instance == nullptr) {
        instance = new Caffeine;
    }
    return instance;
}

Caffeine::~Caffeine()
{
    setActive(false);
}

} // namespace mobilesacn
