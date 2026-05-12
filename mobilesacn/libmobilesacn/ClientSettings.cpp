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
#include <QSettings>

static constexpr auto kClientSettingsGroup = QLatin1StringView("clientSettings");

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
    QSettings settings;
    settings.beginGroup(kClientSettingsGroup);

    // Preferred color mode
    {
        const auto preferredColorMode = settings.value(kSettingPreferredColorScheme);
        if (preferredColorMode.isValid()
            && kAllowedPreferredColorScheme.contains(preferredColorMode.toString())) {
            preferredColorMode_ = preferredColorMode.toString();
        }
    }

    // Level display mode
    {
        const auto levelDisplayMode = settings.value(kSettingLevelDisplayMode);
        if (levelDisplayMode.isValid()
            && kAllowedLevelDisplayMode.contains(levelDisplayMode.toString())) {
            levelDisplayMode_ = levelDisplayMode.toString();
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
    QSettings settings;
    settings.beginGroup(kClientSettingsGroup);

    // Preferred color mode
    if (preferredColorMode_.has_value()) {
        settings.setValue(kSettingPreferredColorScheme, *preferredColorMode_);
    } else {
        settings.remove(kSettingPreferredColorScheme);
    }

    // Level display mode
    if (levelDisplayMode_.has_value()) {
        settings.setValue(kSettingLevelDisplayMode, *levelDisplayMode_);
    } else {
        settings.remove(kSettingLevelDisplayMode);
    }
}

} // namespace mobilesacn
