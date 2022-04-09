/**
 * @file MainWindow.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "MainWindow.h"
#include <QSplitter>
#include <QFormLayout>
#include <QPushButton>
#include "NetIntModel.h"

namespace mobilesacn {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  InitUi();
  SCurrentWebUiIfaceChanged(widgets_.webui_iface_select->currentIndex());
  SCurrentSacnIfaceChanged(widgets_.sacn_iface_select->currentIndex());
}

void MainWindow::InitUi() {
  auto *splitter = new QSplitter(Qt::Horizontal, this);
  setCentralWidget(splitter);

  // Config form
  auto *config_widget = new QWidget(this);
  auto *config_layout = new QFormLayout(config_widget);
  // Web ui interface
  widgets_.webui_iface_select = new QComboBox(config_widget);
  widgets_.webui_iface_select_model = new NetIntListModel(widgets_.webui_iface_select);
  widgets_.webui_iface_select->setModel(widgets_.webui_iface_select_model);
  widgets_.webui_iface_select->setCurrentIndex(widgets_.webui_iface_select_model->GetDefaultRow());
  config_layout->addRow(tr("Web UI Interface"), widgets_.webui_iface_select);
  connect(widgets_.webui_iface_select, &QComboBox::currentIndexChanged, this, &MainWindow::SCurrentWebUiIfaceChanged);
  // sACN interface
  widgets_.sacn_iface_select = new QComboBox(config_widget);
  widgets_.sacn_iface_select_model = new NetIntListModel(widgets_.sacn_iface_select);
  widgets_.sacn_iface_select->setModel(widgets_.sacn_iface_select_model);
  widgets_.sacn_iface_select->setCurrentIndex(widgets_.sacn_iface_select_model->GetDefaultRow());
  config_layout->addRow(tr("sACN Interface"), widgets_.sacn_iface_select);
  connect(widgets_.sacn_iface_select, &QComboBox::currentIndexChanged, this, &MainWindow::SCurrentSacnIfaceChanged);
  // Start button
  widgets_.start_button = new QPushButton(config_widget);
  UpdateStartButtonText();
  config_layout->addWidget(widgets_.start_button);
  connect(widgets_.start_button, &QPushButton::clicked, this, &MainWindow::SStartStopApp);

  splitter->addWidget(config_widget);
}

void MainWindow::UpdateStartButtonText() {
//  if (app_.IsRunning()) {
//    widgets_.start_button->setText(tr("Stop"));
//  } else {
  widgets_.start_button->setText(tr("Start"));
//  }
}

void MainWindow::SStartStopApp() {
  app_.Run(app_options_);
  UpdateStartButtonText();
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
