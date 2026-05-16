/**
 * @file main.cpp
 *
 * @author Dan Keenan
 * @date 3/25/22
 * @copyright Apache-2.0
 */

#include "../libmobilesacn/Settings.h"
#include "MainWindow.h"
#include "log_files.h"
#include "mobilesacn_config.h"
#include <chrono>
#include <etcpal/cpp/common.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUuid>
#ifdef SENTRY_DSN
#include "SentryLogSink.h"
#include <sentry.h>
#endif

using namespace mobilesacn;

static constexpr auto kEtcPalFeatures = ETCPAL_FEATURE_LOGGING | ETCPAL_FEATURE_NETINTS;

void setup_logging()
{
    // Cap log file size at 1MB before rotating.  Keep at most 5 log files.
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        get_log_path(), 1024 * 1024, 5, true);
    file_sink->set_level(spdlog::level::debug);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    spdlog::default_logger()->sinks() = {stdout_sink, file_sink};
    spdlog::default_logger()->set_level(spdlog::level::debug);
    spdlog::default_logger()->flush_on(spdlog::level::warn);
}

void setup_sentry()
{
#ifdef SENTRY_DSN
    // Set options.
    sentry_options_t *options = sentry_options_new();
    const auto handler_path = std::filesystem::path(qApp->applicationDirPath().toStdString())
                              / CRASHPAD_HANDLER_FILENAME;
    sentry_options_set_handler_path(options, handler_path.string().c_str());
    const auto db_path
        = std::filesystem::path(
              QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString())
          / "sentry";
    sentry_options_set_database_path(options, db_path.string().c_str());
    sentry_options_set_dsn(options, SENTRY_DSN);
    sentry_options_set_release(options, config::kProjectCommitSha);

    // Send logged messages to Sentry.
    auto sentry_sink = std::make_shared<SentryLogSink<std::mutex>>(spdlog::level::err);
    sentry_sink->set_level(spdlog::level::err);
    spdlog::default_logger()->sinks().push_back(sentry_sink);

    sentry_init(options);

    // App context.
    sentry_value_t context_app = sentry_value_new_object();
    const auto now = fmt::format("{:%Y-%m-%dT%H:%M:%S%z}", std::chrono::system_clock::now());
    sentry_value_set_by_key(context_app, "app_start_time", sentry_value_new_string(now.c_str()));
    sentry_value_set_by_key(
        context_app, "app_name", sentry_value_new_string(config::kProjectDisplayName));
    sentry_value_set_by_key(
        context_app, "app_version", sentry_value_new_string(config::kProjectVersion));
    const auto build_timestamp_str = fmt::to_string(config::kProjectBuildTimestamp);
    sentry_value_set_by_key(
        context_app, "app_build_timestamp", sentry_value_new_string(build_timestamp_str.c_str()));
    sentry_set_context("app", context_app);
#endif
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(mobilesacn::config::kProjectOrganizationName);
    app.setOrganizationDomain(mobilesacn::config::kProjectOrganizationDomain);
    app.setApplicationName(mobilesacn::config::kProjectName);
    app.setApplicationDisplayName(mobilesacn::config::kProjectDisplayName);
    app.setApplicationVersion(mobilesacn::config::kProjectVersion);
    app.setWindowIcon(QIcon(":/logo.svg"));

    setup_logging();
    setup_sentry();

    // Init EtcPal.
    etcpal_init(kEtcPalFeatures);

    MainWindow main_window;
    main_window.show();

    const auto ret = QApplication::exec();

    // Deinit etcpal.
    etcpal_deinit(kEtcPalFeatures);

#ifdef SENTRY_DSN
    sentry_close();
#endif

    return ret;
}
