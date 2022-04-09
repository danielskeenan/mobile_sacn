/**
 * @file LogViewer.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "LogViewer.h"
#include <QVBoxLayout>
#include <fmt/format.h>
#include <QDateTime>
#include <QLocale>

namespace mobilesacn {

LogViewer::LogViewer(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);

  widgets_.text_view = new QPlainTextEdit(this);
  widgets_.text_view->setReadOnly(true);
  widgets_.text_view->setCenterOnScroll(true);
  widgets_.text_view->setStyleSheet(R"(
QPlainTextEdit {
  background: #00007F;
  font-family: monospace;
};
)");
  layout->addWidget(widgets_.text_view);
}

void LogViewer::SLog(const QString &msg) {
  widgets_.text_view->appendHtml(msg);
}

void WidgetLogFormatter::format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) {
  const auto timestamp = QDateTime::fromSecsSinceEpoch(msg.time.time_since_epoch().count());
  fmt::format_to(std::back_inserter(dest),
                 R"(<p style="{log_style}">{timestamp} [{level}] {message}</p>)",
                 fmt::arg("log_style", LogStyle(msg.level)),
                 fmt::arg("timestamp", QLocale().toString(timestamp, QLocale::ShortFormat).toStdString()),
                 fmt::arg("level", spdlog::level::to_string_view(msg.level)),
                 fmt::arg("message", msg.payload)
  );
}

const char *WidgetLogFormatter::LogStyle(spdlog::level::level_enum level) {
  switch (level) {
    case spdlog::level::trace:
    case spdlog::level::debug:return "font-style: normal; font-weight: normal; color: #777777;";
    case spdlog::level::info:return "font-style: normal; font-weight: normal; color: #CCCCCC;";
    case spdlog::level::warn:return "font-style: normal; font-weight: normal; color: #FF7F00;";
    case spdlog::level::err:return "font-style: normal; font-weight: normal; color: #FF0000;";
    case spdlog::level::critical:return "font-style: normal; font-weight: bold; color: #FF0000;";
    default:return "";
  }
}
} // mobilesacn
