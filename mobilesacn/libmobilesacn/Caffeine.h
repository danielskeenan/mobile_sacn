/**
 * @file Caffeine.h
 *
 * @author Dan Keenan
 * @date 11/30/25
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_CAFFEINE_H
#define MOBILESACN_LIBMOBILESACN_CAFFEINE_H

namespace mobilesacn {

/**
 * Prevent the system from sleeping.
 */
class Caffeine
{
public:
    static Caffeine *get();
    ~Caffeine();

    [[nodiscard]] bool isActive() const { return active_; }
    void setActive(bool active);

private:
    bool active_ = false;
#ifdef PLATFORM_LINUX
    int inhibitHandle_ = -1;
#endif
};

} // namespace mobilesacn

#endif //MOBILESACN_LIBMOBILESACN_CAFFEINE_H
