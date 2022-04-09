/**
 * @file MainWindow.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "MainWindow.h"
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include "NetIntModel.h"
#include <QApplication>

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
  resize(1024, 600);

  auto central = new QWidget(this);
  setCentralWidget(central);
  auto layout = new QHBoxLayout(central);

  // Config form
  auto *config_widget = new QWidget(this);
  auto *config_layout = new QFormLayout(config_widget);
  // Web ui interface
  widgets_.webui_iface_select = new QComboBox(config_widget);
  widgets_.webui_iface_select_model = new NetIntListModel(widgets_.webui_iface_select);
  widgets_.webui_iface_select->setModel(widgets_.webui_iface_select_model);
  widgets_.webui_iface_select->setCurrentIndex(widgets_.webui_iface_select_model->GetDefaultRow());
  config_layout->addRow(tr("Web UI Interface"), widgets_.webui_iface_select);
  connect(widgets_.webui_iface_select,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &MainWindow::SCurrentWebUiIfaceChanged);
  // sACN interface
  widgets_.sacn_iface_select = new QComboBox(config_widget);
  widgets_.sacn_iface_select_model = new NetIntListModel(widgets_.sacn_iface_select);
  widgets_.sacn_iface_select->setModel(widgets_.sacn_iface_select_model);
  widgets_.sacn_iface_select->setCurrentIndex(widgets_.sacn_iface_select_model->GetDefaultRow());
  config_layout->addRow(tr("sACN Interface"), widgets_.sacn_iface_select);
  connect(widgets_.sacn_iface_select,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &MainWindow::SCurrentSacnIfaceChanged);
  // Start button
  widgets_.start_button = new QPushButton(config_widget);
  config_layout->addWidget(widgets_.start_button);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartApp);
  SAppStopped();
  layout->addWidget(config_widget);

  // Log viewer
  widgets_.log_viewer = new LogViewer(this);
  layout->addWidget(widgets_.log_viewer);
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
}

void MainWindow::SAppStopped() {
  widgets_.start_button->setText(tr("Start"));
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

} // mobilesacn
