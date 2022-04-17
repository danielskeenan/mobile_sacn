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
#include "Settings.h"

namespace mobilesacn {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  InitUi();
  SCurrentWebUiIfaceChanged(widgets_.webui_iface_select->currentIndex());
  SCurrentSacnIfaceChanged(widgets_.sacn_iface_select->currentIndex());

  // Tell spdlog to log to this window.
  auto widget_log_sink = std::make_shared<WidgetLogSink<std::mutex>>(widgets_.log_viewer);
  auto widget_logger = std::make_shared<spdlog::logger>(qApp->applicationDisplayName().toStdString(), widget_log_sink);
  spdlog::register_logger(widget_logger);
  spdlog::set_default_logger(widget_logger);
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
  // Start button
  widgets_.start_button = new QPushButton(this);
  config_form->addWidget(widgets_.start_button);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartApp);
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
