/**
 * @file SentryLogSink.h
 *
 * @author Dan Keenan
 * @date 5/29/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_SENTRYLOGSINK_H_
#define MOBILE_SACN_SRC_MOBILESACN_SENTRYLOGSINK_H_

#include <spdlog/sinks/base_sink.h>
#include <sentry.h>

namespace mobilesacn {

/**
 * Send log messages to Sentry.
 * @tparam Mutex
 */
template<typename Mutex>
class SentryLogSink : public spdlog::sinks::base_sink<Mutex> {
 public:
  /**
   * @param event_level The level to consider an Event.  All messages *below* this level are added as breadcrumbs.
   * All messages *at or above* this level are sent to Sentry as Events.
   */
  explicit SentryLogSink(spdlog::level::level_enum event_level) : event_level_(event_level) {}

 protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    if (msg.level >= event_level_) {
      CaptureEvent(msg);
    } else {
      CaptureBreadcrumb(msg);
    }
  }

  void flush_() override {
    sentry_flush(1000);
  }

 private:
  spdlog::level::level_enum event_level_;

  static sentry_level_t SentryLevelForSpdLogLevel(spdlog::level::level_enum level) {
    switch (level) {
      case spdlog::level::trace:
      case spdlog::level::debug:return SENTRY_LEVEL_DEBUG;
      case spdlog::level::info:return SENTRY_LEVEL_INFO;
      case spdlog::level::warn:return SENTRY_LEVEL_WARNING;
      case spdlog::level::err:return SENTRY_LEVEL_ERROR;
      case spdlog::level::critical:return SENTRY_LEVEL_FATAL;
      default: return SENTRY_LEVEL_ERROR;
    }
    return SENTRY_LEVEL_ERROR;
  }

  static std::string SentryLevelNameForSpdLogLevel(spdlog::level::level_enum level) {
    switch (SentryLevelForSpdLogLevel(level)) {
      case SENTRY_LEVEL_DEBUG: return "debug";
      case SENTRY_LEVEL_INFO: return "info";
      case SENTRY_LEVEL_WARNING:return "warning";
      case SENTRY_LEVEL_ERROR:return "error";
      case SENTRY_LEVEL_FATAL: return "fatal";
    }
    return "error";
  }

  [[nodiscard]] static std::string LoggerName(const spdlog::details::log_msg &msg) {
    return fmt::format("spdlog-{}", msg.logger_name);
  }

  static std::string LogMessage(const spdlog::details::log_msg &msg) {
    return {msg.payload.begin(), msg.payload.end()};
  }

  void CaptureEvent(const spdlog::details::log_msg &msg) const {
    sentry_value_t event = sentry_value_new_message_event(
        SentryLevelForSpdLogLevel(msg.level),
        LoggerName(msg).c_str(),
        LogMessage(msg).c_str()
    );
    sentry_capture_event(event);
  }

  void CaptureBreadcrumb(const spdlog::details::log_msg &msg) const {
    sentry_value_t crumb = sentry_value_new_breadcrumb("default", LogMessage(msg).c_str());
    sentry_value_set_by_key(crumb, "category", sentry_value_new_string("console"));
    sentry_value_set_by_key(crumb, "level", sentry_value_new_string(SentryLevelNameForSpdLogLevel(msg.level).c_str()));
    sentry_add_breadcrumb(crumb);
  }
};

} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_SENTRYLOGSINK_H_
