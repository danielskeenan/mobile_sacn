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

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(mobilesacn::config::kProjectOrganizationName);
  app.setOrganizationDomain(mobilesacn::config::kProjectOrganizationDomain);
  app.setApplicationName(mobilesacn::config::kProjectName);
  app.setApplicationDisplayName(mobilesacn::config::kProjectDisplayName);
  app.setApplicationVersion(mobilesacn::config::kProjectVersion);
//  app.setWindowIcon(QIcon(":/app-icon"));

  // Icon theme, for non Linux platforms
#ifndef PLATFORM_LINUX
  QIcon::setThemeName("breeze-light");
#endif
  
  MainWindow main_window;
  main_window.show();

  return QApplication::exec();
}
