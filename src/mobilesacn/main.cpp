/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright GNU GPLv3
 */

#include <QApplication>
#ifdef SENTRY_DSN
#include <sentry.h>
#include "SentryLogSink.h"
#endif
#include "mobilesacn_config.h"
#include "MainWindow.h"
#include "updater/updater.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include "log_files.h"
#include "Settings.h"
#include <QUuid>
#include <etcpal/cpp/common.h>
#include <chrono>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <QStandardPaths>
#include <QMessageBox>

using namespace mobilesacn;

static constexpr auto kEtcPalFeatures = ETCPAL_FEATURE_LOGGING | ETCPAL_FEATURE_NETINTS;

void setup_logging() {
  // Cap log file size at 1MB before rotating.  Keep at most 5 log files.
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(get_log_path(), 1024 * 1024, 5, true);
  file_sink->set_level(spdlog::level::debug);
  spdlog::default_logger()->sinks() = {file_sink};
  spdlog::default_logger()->set_level(spdlog::level::debug);
  spdlog::default_logger()->flush_on(spdlog::level::warn);
}

void setup_sentry() {
#ifdef SENTRY_DSN
  // Set options.
  sentry_options_t *options = sentry_options_new();
  const auto db_path =
      std::filesystem::path(QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString()) / "sentry";
  sentry_options_set_database_path(options, db_path.string().c_str());
  sentry_options_set_dsn(options, SENTRY_DSN);
  sentry_options_set_release(options, config::kProjectCommitSha);

  // Send logged messages to Sentry.
  auto sentry_sink = std::make_shared<SentryLogSink<std::mutex>>(spdlog::level::err);
  sentry_sink->set_level(spdlog::level::err);
  spdlog::default_logger()->sinks().push_back(sentry_sink);

  sentry_init(options);

  // User id.
  auto user_id = Settings::GetUserId();
  if (user_id.isEmpty()) {
    user_id = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    Settings::SetUserId(user_id);
  }
  sentry_value_t user = sentry_value_new_object();
  sentry_value_set_by_key(user, "id", sentry_value_new_string(user_id.toStdString().c_str()));
  sentry_set_user(user);

  // App context.
  sentry_value_t context_app = sentry_value_new_object();
  const auto now = fmt::format("{:%Y-%m-%dT%H:%M:%S%z}", fmt::gmtime(std::chrono::system_clock::now()));
  sentry_value_set_by_key(context_app, "app_start_time", sentry_value_new_string(now.c_str()));
  sentry_value_set_by_key(context_app, "app_name", sentry_value_new_string(config::kProjectDisplayName));
  sentry_value_set_by_key(context_app, "app_version", sentry_value_new_string(config::kProjectVersion));
  const auto build_timestamp_str = fmt::to_string(config::kProjectBuildTimestamp);
  sentry_value_set_by_key(context_app, "app_build", sentry_value_new_string(build_timestamp_str.c_str()));
  sentry_set_context("app", context_app);
#endif
}

/**
 * Ask the user about resetting settings
 * @param app The QApplication instance with an installed translator
 */
bool ReallyClearSettings(QApplication &app) {
  auto *dialog = new QMessageBox(
      QMessageBox::Question,
      app.translate("entrypoint", "Clear settings?"),
      app.translate("entrypoint", "Holding SHIFT while launching this program will clear all settings.\n"
                                  "Are you sure you wish to reset the settings to their defaults?"),
      QMessageBox::Yes | QMessageBox::No);
  dialog->setDefaultButton(QMessageBox::No);
  return dialog->exec() == QMessageBox::Yes;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(mobilesacn::config::kProjectOrganizationName);
  app.setOrganizationDomain(mobilesacn::config::kProjectOrganizationDomain);
  app.setApplicationName(mobilesacn::config::kProjectName);
  app.setApplicationDisplayName(mobilesacn::config::kProjectDisplayName);
  app.setApplicationVersion(mobilesacn::config::kProjectVersion);
  app.setWindowIcon(QIcon(":/logo.svg"));

  // Clear all settings if program is launched while holding [Shift].
  if (app.queryKeyboardModifiers() == Qt::ShiftModifier) {
    if (ReallyClearSettings(app)) {
      Settings::Clear();
    }
  }

  setup_logging();
  setup_sentry();

  // Init EtcPal.
  etcpal_init(kEtcPalFeatures);

  MainWindow main_window;
  main_window.show();

  init_updater();

  const auto ret = QApplication::exec();

  // Deinit etcpal.
  etcpal_deinit(kEtcPalFeatures);

  cleanup_updater();

#ifdef SENTRY_DSN
  sentry_close();
#endif

  return ret;
}
