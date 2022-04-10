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
#include <fmt/chrono.h>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>
#include <QOperatingSystemVersion>
#include <QFontDatabase>
#include <QFont>
#include "mobilesacn_config.h"

namespace mobilesacn {

LogViewer::LogViewer(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);

  widgets_.text_view = new QPlainTextEdit(this);
  widgets_.text_view->setReadOnly(true);
  const auto monospace_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  widgets_.text_view->setStyleSheet(QStringLiteral(R"(
QPlainTextEdit {
  background: #00007F;
  font-family: %1;
};
)").arg(monospace_font.family()));
  layout->addWidget(widgets_.text_view);

  // Context menu
  widgets_.text_view->setContextMenuPolicy(Qt::ActionsContextMenu);
  // Copy
  actions_.act_copy = new QAction(tr("Copy"), widgets_.text_view);
  connect(widgets_.text_view, &QPlainTextEdit::copyAvailable, actions_.act_copy, &QAction::setEnabled);
  widgets_.text_view->addAction(actions_.act_copy);
  connect(actions_.act_copy, &QAction::triggered, this, &LogViewer::SCopy);
  // Save logs
  actions_.act_save_logs = new QAction(tr("Save Logs..."), widgets_.text_view);
  widgets_.text_view->addAction(actions_.act_save_logs);
  connect(actions_.act_save_logs, &QAction::triggered, this, &LogViewer::SSaveLogs);
}

void LogViewer::SLog(const QString &msg) {
  widgets_.text_view->appendHtml(msg);
}

void LogViewer::SCopy() {
  widgets_.text_view->copy();
}

void LogViewer::SSaveLogs() {
  // TODO: Save all logs down to debug level to a file and copy it with this option.
  const auto filename = QFileDialog::getSaveFileName(
      this,
      tr("Save logs"),
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
      tr("Log files (*.log)")
  );
  if (filename.isEmpty()) {
    return;
  }

  QFile out_file(filename, this);
  if (!out_file.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, tr("Error saving log file"), tr("The file could not be written."));
    return;
  }
  QTextStream out(&out_file);
  out << config::kProjectName << " (" << config::kProjectVersion << ")\n"
      << QSysInfo::currentCpuArchitecture() << " "
      << QSysInfo::productType() << " "
      << QSysInfo::productVersion() << " ("
      << QOperatingSystemVersion::current().name() << ")\n"
      << "\n"
      << widgets_.text_view->toPlainText();
  out.flush();
  out_file.close();
}

void WidgetLogFormatter::format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) {
  fmt::format_to(std::back_inserter(dest),
                 R"(<p style="{log_style}">{timestamp} [{level}] {message}</p>)",
                 fmt::arg("log_style", LogStyle(msg.level)),
                 fmt::arg("timestamp", msg.time),
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
