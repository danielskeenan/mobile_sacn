/**
 * @file LogViewer.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_LOGVIEWER_H_
#define MOBILE_SACN_SRC_MOBILESACN_LOGVIEWER_H_

#include <QPlainTextEdit>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

namespace mobilesacn {

/**
 * Display log messages.
 */
class LogViewer : public QWidget {
 Q_OBJECT
 public:
  explicit LogViewer(QWidget *parent = nullptr);

 public Q_SLOTS:
  void SLog(const QString &msg);

 private:
  struct Widgets {
    QPlainTextEdit *text_view = nullptr;
  };
  Widgets widgets_;
};

/**
 * Formatter for logs in a LogViewer widget.
 */
class WidgetLogFormatter : public spdlog::formatter {
 public:
  void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override;

  [[nodiscard]] std::unique_ptr<formatter> clone() const override {
    return std::make_unique<WidgetLogFormatter>();
  }

 private:
  static const char *LogStyle(spdlog::level::level_enum level);
};

/**
 * spdlog sink that shows log messages in a LogViewer widget.
 * @tparam Mutex
 */
template<typename Mutex>
class WidgetLogSink : public spdlog::sinks::base_sink<Mutex> {
 public:
  explicit WidgetLogSink(LogViewer *log_viewer) : log_viewer_(log_viewer) {
    this->set_formatter(std::make_unique<WidgetLogFormatter>());
  }

 protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    WidgetLogSink::formatter_->format(msg, formatted);
    spdlog::string_view_t str(formatted.data(), formatted.size());
    QMetaObject::invokeMethod(
        log_viewer_, "SLog", Qt::AutoConnection,
        Q_ARG(QString, QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed()));
  }

  void flush_() override {}

 private:
  LogViewer *log_viewer_;
};

} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_LOGVIEWER_H_
