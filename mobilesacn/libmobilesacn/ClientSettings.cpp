/**
 * @file ClientSettings.cpp
 *
 * @author Dan Keenan
 * @date 3/28/26
 * @copyright Apache-2.0
 */

#include "ClientSettings.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include "Settings.h"

static constexpr auto kSettingPreferredColorScheme = QLatin1StringView("preferredColorScheme");
static const QSet kAllowedPreferredColorScheme{
    QStringLiteral(""),
    QStringLiteral("light"),
    QStringLiteral("dark"),
};

static constexpr auto kSettingLevelDisplayMode = QLatin1StringView("levelDisplayMode");
static const QSet kAllowedLevelDisplayMode{
    QStringLiteral("decimal"),
    QStringLiteral("hex"),
    QStringLiteral("percent"),
};

namespace mobilesacn {

ClientSettings::ClientSettings()
{
    // Preferred color mode
    {
        const auto preferredColorMode = Settings::getPreferredColorScheme();
        if (kAllowedPreferredColorScheme.contains(preferredColorMode)) {
            preferredColorMode_ = preferredColorMode;
        }
    }

    // Level display mode
    {
        const auto levelDisplayMode = Settings::getLevelDisplayMode();
        if (kAllowedLevelDisplayMode.contains(levelDisplayMode)) {
            levelDisplayMode_ = levelDisplayMode;
        }
    }
}

ClientSettings::ClientSettings(const QJsonDocument &json)
{
    // Preferred color mode
    {
        const auto preferredColorScheme = json[kSettingPreferredColorScheme].toString();
        if (kAllowedPreferredColorScheme.contains(preferredColorScheme)) {
            preferredColorMode_ = preferredColorScheme;
        }
    }

    // Level display mode
    {
        const auto levelDisplayMode = json[kSettingLevelDisplayMode].toString();
        if (kAllowedLevelDisplayMode.contains(levelDisplayMode)) {
            levelDisplayMode_ = levelDisplayMode;
        }
    }
}

QJsonObject ClientSettings::toJson() const
{
    QJsonObject json;

    // Preferred color mode
    if (preferredColorMode_.has_value()) {
        json[kSettingPreferredColorScheme] = *preferredColorMode_;
    }

    // Level display mode
    if (levelDisplayMode_.has_value()) {
        json[kSettingLevelDisplayMode] = *levelDisplayMode_;
    }

    return json;
}

void ClientSettings::save() const
{
    // Preferred color mode
    if (preferredColorMode_.has_value()) {
        Settings::setPreferredColorScheme(*preferredColorMode_);
    } else {
        Settings::unsetPreferredColorScheme();
    }

    // Level display mode
    if (levelDisplayMode_.has_value()) {
        Settings::setLevelDisplayMode(*levelDisplayMode_);
    } else {
        Settings::unsetLevelDisplayMode();
    }

    Settings::sync();
}

} // namespace mobilesacn
