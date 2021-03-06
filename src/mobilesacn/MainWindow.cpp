/**
 * @file MainWindow.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include "NetIntModel.h"
#include <QApplication>
#include <QProcess>
#include <QStandardPaths>
#include "Settings.h"
#include "mobilesacn_config.h"

namespace mobilesacn {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  InitUi();
  SCurrentWebUiIfaceChanged(widgets_.webui_iface_select->currentIndex());
  SCurrentSacnIfaceChanged(widgets_.sacn_iface_select->currentIndex());

  // Tell spdlog to log to this window.
  auto widget_log_sink = std::make_shared<WidgetLogSink<std::mutex>>(widgets_.log_viewer);
  widget_log_sink->set_level(spdlog::level::info);
  spdlog::default_logger()->sinks().push_back(widget_log_sink);
}

void MainWindow::InitUi() {
  if (!restoreGeometry(Settings::GetMainWindowGeometry())) {
    resize(1024, 600);
    Settings::SetMainWindowGeometry(saveGeometry());
  }

  auto central = new QWidget(this);
  setCentralWidget(central);
  auto layout = new QHBoxLayout(central);
  auto sidebar_layout = new QVBoxLayout;
  layout->addLayout(sidebar_layout);

  // Config form
  auto *config_form = new QFormLayout;
  sidebar_layout->addLayout(config_form);

  // Web ui interface
  widgets_.webui_iface_select = new QComboBox(this);
  widgets_.webui_iface_select_model = new NetIntListModel(widgets_.webui_iface_select);
  widgets_.webui_iface_select->setModel(widgets_.webui_iface_select_model);
  const QString &last_web_ui_interface_name = Settings::GetLastWebUiInterfaceName();
  if (last_web_ui_interface_name.isEmpty()) {
    const auto default_iface_row = widgets_.webui_iface_select_model->GetDefaultRow();
    widgets_.webui_iface_select->setCurrentIndex(default_iface_row);
    Settings::SetLastWebUiInterfaceName(QString::fromStdString(
        widgets_.webui_iface_select_model->GetNetIntInfo(default_iface_row).GetFriendlyName()));
  } else {
    widgets_.webui_iface_select->setCurrentIndex(widgets_.webui_iface_select_model->GetRowForInterfaceName(
        last_web_ui_interface_name.toStdString()));
  }
  config_form->addRow(tr("Web UI Interface"), widgets_.webui_iface_select);
  connect(widgets_.webui_iface_select,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &MainWindow::SCurrentWebUiIfaceChanged);

  // sACN interface
  widgets_.sacn_iface_select = new QComboBox(this);
  widgets_.sacn_iface_select_model = new NetIntListModel(widgets_.sacn_iface_select);
  widgets_.sacn_iface_select->setModel(widgets_.sacn_iface_select_model);
  const QString &last_sacn_interface_name = Settings::GetLastSacnInterfaceName();
  if (last_sacn_interface_name.isEmpty()) {
    const auto default_iface_row = widgets_.sacn_iface_select_model->GetDefaultRow();
    widgets_.sacn_iface_select->setCurrentIndex(default_iface_row);
    Settings::SetLastSacnInterfaceName(QString::fromStdString(
        widgets_.sacn_iface_select_model->GetNetIntInfo(default_iface_row).GetFriendlyName()));
  } else {
    widgets_.sacn_iface_select->setCurrentIndex(widgets_.sacn_iface_select_model->GetRowForInterfaceName(
        last_sacn_interface_name.toStdString()));
  }
  config_form->addRow(tr("sACN Interface"), widgets_.sacn_iface_select);
  connect(widgets_.sacn_iface_select,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &MainWindow::SCurrentSacnIfaceChanged);

  auto *buttons_layout = new QHBoxLayout;
  // Start button
  widgets_.start_button = new QPushButton(this);
  buttons_layout->addWidget(widgets_.start_button);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartApp);

  // Help button
  auto *help_button = new QPushButton(tr("Help"), this);
  buttons_layout->addWidget(help_button);
  connect(help_button, &QPushButton::clicked, this, &MainWindow::SHelp);

  config_form->addRow(buttons_layout);
  sidebar_layout->addStretch();

  // QR code
  auto *qr_layout = new QHBoxLayout;
  qr_layout->setAlignment(Qt::AlignCenter);
  sidebar_layout->addLayout(qr_layout);
  widgets_.qr_code = new QrCode(this);
  qr_layout->addWidget(widgets_.qr_code);
  sidebar_layout->addStretch();

  // Log viewer
  widgets_.log_viewer = new LogViewer(this);
  layout->addWidget(widgets_.log_viewer);

  SAppStopped();
}

void MainWindow::SStartApp() {
  app_.Run(app_options_);
  disconnect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartApp);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStopApp);
  SAppStarted();
}

void MainWindow::SStopApp() {
  app_.Stop();
  disconnect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStopApp);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartApp);
  SAppStopped();
}

void MainWindow::SHelp() {
  const std::filesystem::path app_dir_path(qApp->applicationDirPath().toStdString());
  const auto collection_file = app_dir_path.parent_path() / config::kHelpPath
      / fmt::format("{}.qhc", config::kProjectName);
  const auto assistant_path = [&app_dir_path]() {
    for (const auto &assistant_name : {"assistant", "Assistant"}) {
      auto found_path = QStandardPaths::findExecutable(assistant_name);
      if (!found_path.isEmpty()) {
        return found_path;
      }
#ifdef PLATFORM_MACOS
      const auto extra_path = QString::fromStdString((app_dir_path.parent_path() / "Resources" / "Assistant.app"
          / "Contents" / "MacOS").string());
#else
      const auto extra_path = qApp->applicationDirPath();
#endif
      found_path = QStandardPaths::findExecutable(assistant_name, {extra_path});
      if (!found_path.isEmpty()) {
        return found_path;
      }
    }
    return QString();
  }();

  auto *process = new QProcess(this);
  connect(process, &QProcess::readyRead, [process]() {
    while (process->canReadLine()) {
      spdlog::debug(QString::fromUtf8(process->readLine()).toStdString());
    }
  });
  QStringList args(
      {
          "-collectionFile",
          QString::fromStdString(collection_file.string()),
      });
  process->start(assistant_path, args);
  process->waitForStarted();
}

void MainWindow::SAppStarted() {
  widgets_.start_button->setText(tr("Stop"));
  widgets_.qr_code->SetContents(app_.GetWebUrl());
}

void MainWindow::SAppStopped() {
  widgets_.start_button->setText(tr("Start"));
  widgets_.qr_code->Clear();
}

void MainWindow::SCurrentWebUiIfaceChanged(int row) {
  const auto &iface = widgets_.webui_iface_select_model->GetNetIntInfo(row);
  // TODO: Pass an iface index.
  app_options_.backend_address = iface.GetAddr().ToString();
}

void MainWindow::SCurrentSacnIfaceChanged(int row) {
  const auto &iface = widgets_.sacn_iface_select_model->GetNetIntInfo(row);
  // TODO: Pass an iface index.
  app_options_.sacn_address = iface.GetAddr();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  SStopApp();
  Settings::SetMainWindowGeometry(saveGeometry());
  Settings::Sync();
  event->accept();
}

} // mobilesacn
