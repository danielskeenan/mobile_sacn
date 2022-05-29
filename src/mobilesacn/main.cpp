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
#include "updater/updater.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include "log_files.h"

using namespace mobilesacn;

static const auto kEtcPalFeatures = ETCPAL_FEATURE_LOGGING | ETCPAL_FEATURE_NETINTS;

void setup_logging() {
  // Cap log file size at 1MB before rotating.  Keep at most 5 log files.
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(get_log_path(), 1024*1024, 5, true);
  file_sink->set_level(spdlog::level::debug);
  spdlog::default_logger()->sinks() = {file_sink};
  spdlog::default_logger()->set_level(spdlog::level::debug);
  spdlog::default_logger()->flush_on(spdlog::level::warn);
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(mobilesacn::config::kProjectOrganizationName);
  app.setOrganizationDomain(mobilesacn::config::kProjectOrganizationDomain);
  app.setApplicationName(mobilesacn::config::kProjectName);
  app.setApplicationDisplayName(mobilesacn::config::kProjectDisplayName);
  app.setApplicationVersion(mobilesacn::config::kProjectVersion);
  app.setWindowIcon(QIcon(":/logo.svg"));

  setup_logging();

  // Init EtcPal.
  etcpal_init(kEtcPalFeatures);

  MainWindow main_window;
  main_window.show();

  init_updater();

  const auto ret = QApplication::exec();

  // Deinit etcpal.
  etcpal_deinit(kEtcPalFeatures);

  cleanup_updater();

  return ret;
}
