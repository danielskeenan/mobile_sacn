/**
 * @file SacnSettings.h
 *
 * @author Dan Keenan
 * @date 9/14/24
 * @copyright GNU GPLv3
 */

#ifndef SACNSETTINGS_H
#define SACNSETTINGS_H

#include <vector>
#include <etcpal/cpp/inet.h>

namespace mobilesacn {
class SacnSettings;

namespace detail {
inline SacnSettings* sacnSettingsInstance = nullptr;
} // detail

class SacnSettings
{
public:
    SacnSettings(const SacnSettings&) = delete;
    SacnSettings& operator=(const SacnSettings&) = delete;

    etcpal::NetintInfo sacnNetInt{};
    std::vector<SacnMcastInterface> sacnMcastInterfaces{};

    static SacnSettings* getMutable()
    {
        if (detail::sacnSettingsInstance == nullptr) {
            detail::sacnSettingsInstance = new SacnSettings;
        }
        return detail::sacnSettingsInstance;
    }

    static const SacnSettings* get()
    {
        if (detail::sacnSettingsInstance == nullptr) {
            detail::sacnSettingsInstance = new SacnSettings;
        }
        return detail::sacnSettingsInstance;
    }

private:
    explicit SacnSettings() = default;
};

} // mobilesacn

#endif //SACNSETTINGS_H
