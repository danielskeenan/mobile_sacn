/**
 * @file Settings.h
 *
 * @author Dan Keenan
 * @date 4/17/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_
#define MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_

#include <dragoonboots/qsettingscontainer/QSettingsContainer.h>

namespace mobilesacn {

/**
 * Application settings.
 */
class Settings : public dragoonboots::qsettingscontainer::QSettingsContainer {
 public:
  DGSETTINGS_SETTING(QByteArray, MainWindowGeometry, {})

  DGSETTINGS_SETTING(QString, LastWebUiInterfaceName, {})

  DGSETTINGS_SETTING(QString, LastSacnInterfaceName, {})
};

} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_SETTINGS_H_
