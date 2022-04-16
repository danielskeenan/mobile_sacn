/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include <QApplication>
#include "mobilesacn_config.h"
#include "MainWindow.h"

using namespace mobilesacn;

static const auto kEtcPalFeatures = ETCPAL_FEATURE_LOGGING | ETCPAL_FEATURE_NETINTS;

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(mobilesacn::config::kProjectOrganizationName);
  app.setOrganizationDomain(mobilesacn::config::kProjectOrganizationDomain);
  app.setApplicationName(mobilesacn::config::kProjectName);
  app.setApplicationDisplayName(mobilesacn::config::kProjectDisplayName);
  app.setApplicationVersion(mobilesacn::config::kProjectVersion);
  app.setWindowIcon(QIcon(":/logo.svg"));

  // Init EtcPal.
  etcpal_init(kEtcPalFeatures);

  MainWindow main_window;
  main_window.show();

  const auto ret = QApplication::exec();

  // Deinit etcpal.
  etcpal_deinit(kEtcPalFeatures);

  return ret;
}
