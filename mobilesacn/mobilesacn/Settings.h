/**
 * @file Settings.h
 *
 * @author Dan Keenan
 * @date 4/17/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_
#define MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_

#include <QSettings>

/**
 * Similar to @ref MSACN_SETTING, but uses different storage type and value types.
 *
 * @param val_T value type; must be castable using static_cast() to @p storage_t
 * @param storage_T storage type; must be storable in a QVariant
 * @param name Getter/setter name
 * @param default_val Default value to use if no value has previously been
 * stored; must be of type @p val_T
 *
 * @par Example
 * @code{.cpp}
 * MSACN_SETTING_CAST(Breed, unsigned int, PuppyBreed, Breed::Mix)
 * @endcode
 * results in the following method definitions:
 * @code{.cpp}
 * static Breed getPuppyBreed();
 * static void setPuppyBreed(Breed value);
 * static Breed getDefaultPuppyBreed();
 * @endcode
 * casting the value to `unsigned int` when interfacing with QSettings.
 */
#define MSACN_SETTING_CAST(val_T, storage_T, name, default_val) \
    static val_T get##name() \
    { \
        const QVariant val_variant = QSettings().value(#name); \
        if (!val_variant.isValid()) { \
            return default_val; \
        } \
        return static_cast<val_T>(val_variant.value<storage_T>()); \
    } \
    /** Set @ref get##name() */ \
    static void set##name(const val_T &val) \
    { \
        const auto val_storage = static_cast<storage_T>(val); \
        QSettings().setValue(#name, QVariant::fromValue<storage_T>(val_storage)); \
    } \
    /** Default value for @ref get##name() */ \
    static val_T getDefault##name() \
    { \
        return default_val; \
    }

/**
 * Create a new setting that persists between executions.
 *
 * Defines three static methods: `get<name>`, `set<name>`, and `getDefault<name>`.
 *
 * Settings are stored persistently using QSettings.
 *
 * For enum-type variables with a different storage type, see @ref MSACN_SETTING_CAST.
 *
 * @param T value type; must be storable in a QVariant
 * @param name Getter/setter name
 * @param default_val Default value to use if no value has previously been
 * stored; must be of type @p T
 *
 * @par Example
 * @code{.cpp}
 * MSACN_SETTING(int, Puppies, 0)
 * @endcode
 * results in the following method definitions:
 * @code{.cpp}
 * static int GetPuppies();
 * static void SetPuppies(int value);
 * static int GetDefaultPuppies();
 * @endcode
 */
#define MSACN_SETTING(T, name, default_val) MSACN_SETTING_CAST(T, T, name, default_val)

namespace mobilesacn {
/**
 * Application settings.
 */
class Settings
{
    Q_DISABLE_COPY_MOVE(Settings)
public:
    /** Clear all settings */
    static void clear() { QSettings().clear(); }

    /** Sync settings to/from storage */
    static void sync() { QSettings().sync(); }

    MSACN_SETTING(QByteArray, MainWindowGeometry, {})

    MSACN_SETTING(QString, LastWebUiInterfaceName, {})

    MSACN_SETTING(QString, LastSacnInterfaceName, {})

    MSACN_SETTING(QString, UserId, {})
};
} // namespace mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_
