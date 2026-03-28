/**
 * @file ClientSettings.h
 *
 * @author Dan Keenan
 * @date 3/28/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_LIBMOBILESACN_CLIENTSETTINGS_H
#define MOBILESACN_LIBMOBILESACN_CLIENTSETTINGS_H

#include <optional>
#include <QJsonDocument>
#include <QString>

namespace mobilesacn {

/**
 * Store client user settings.
 */
class ClientSettings
{
public:
    /**
     * Load settings from system.
     */
    explicit ClientSettings();

    /**
     * Load settings from @p json.
     * @param json
     */
    explicit ClientSettings(const QJsonDocument &json);

    /**
     * Convert settings to json for sending to client.
     * @return
     */
    [[nodiscard]] QJsonDocument toJson() const;

    /**
     * Save settings to system.
     */
    void save() const;

private:
    std::optional<QString> preferredColorMode_;
    std::optional<QString> levelDisplayMode_;
};

} // namespace mobilesacn

#endif //MOBILESACN_LIBMOBILESACN_CLIENTSETTINGS_H
