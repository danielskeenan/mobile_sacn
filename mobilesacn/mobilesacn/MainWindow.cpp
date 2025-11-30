/**
 * @file MainWindow.cpp
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#include "MainWindow.h"
#include "NetIntModel.h"
#include "Settings.h"
#include "mobilesacn/libmobilesacn/Caffeine.h"
#include "mobilesacn_config.h"
#include <QApplication>
#include <QDesktopServices>
#include <QFormLayout>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace mobilesacn {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), app_(new Application(this))
{
    initUi();
    currentWebUiIfaceChanged(widgets_.webuiIfaceSelect->currentIndex());
    currentSacnIfaceChanged(widgets_.sacnIfaceSelect->currentIndex());

    // Tell spdlog to log to this window.
    auto widget_log_sink = std::make_shared<WidgetLogSink<std::mutex>>(widgets_.logViewer);
    widget_log_sink->set_level(spdlog::level::info);
    spdlog::default_logger()->sinks().push_back(widget_log_sink);
}

void MainWindow::initUi()
{
    if (!restoreGeometry(Settings::GetMainWindowGeometry())) {
        resize(1024, 600);
        Settings::SetMainWindowGeometry(saveGeometry());
    }

    auto central = new QWidget(this);
    setCentralWidget(central);
    auto layout = new QHBoxLayout(central);
    auto sidebarLayout = new QVBoxLayout;
    layout->addLayout(sidebarLayout);

    // Config form
    auto *configForm = new QFormLayout;
    sidebarLayout->addLayout(configForm);

    // Web ui interface
    widgets_.webuiIfaceSelect = new QComboBox(this);
    widgets_.webuiIfaceSelectModel = new NetIntListModel(widgets_.webuiIfaceSelect);
    widgets_.webuiIfaceSelect->setModel(widgets_.webuiIfaceSelectModel);
    const QString &lastWebUiIfaceName = Settings::GetLastWebUiInterfaceName();
    if (lastWebUiIfaceName.isEmpty()) {
        const auto defautIfaceRow = widgets_.webuiIfaceSelectModel->GetDefaultRow();
        widgets_.webuiIfaceSelect->setCurrentIndex(defautIfaceRow);
        const auto defaultIfaceName
            = widgets_.webuiIfaceSelectModel
                  ->data(widgets_.webuiIfaceSelectModel->index(defautIfaceRow, 0, {}), Qt::DisplayRole)
                  .toString();
        Settings::SetLastWebUiInterfaceName(defaultIfaceName);
    } else {
        widgets_.webuiIfaceSelect->setCurrentIndex(
            widgets_.webuiIfaceSelectModel->GetRowForInterfaceName(
                lastWebUiIfaceName.toStdString()));
    }
    configForm->addRow(tr("Web UI Interface"), widgets_.webuiIfaceSelect);
    connect(
        widgets_.webuiIfaceSelect,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &MainWindow::currentWebUiIfaceChanged);

    // sACN interface
    widgets_.sacnIfaceSelect = new QComboBox(this);
    widgets_.sacnIfaceSelectModel = new NetIntListModel(widgets_.sacnIfaceSelect);
    widgets_.sacnIfaceSelect->setModel(widgets_.sacnIfaceSelectModel);
    const QString &lastSacnInterfaceName = Settings::GetLastSacnInterfaceName();
    if (lastSacnInterfaceName.isEmpty()) {
        const auto defautIfaceRow = widgets_.sacnIfaceSelectModel->GetDefaultRow();
        widgets_.sacnIfaceSelect->setCurrentIndex(defautIfaceRow);
        const auto defaultIfaceName
            = widgets_.webuiIfaceSelectModel
                  ->data(widgets_.webuiIfaceSelectModel->index(defautIfaceRow, 0, {}), Qt::DisplayRole)
                  .toString();
        Settings::SetLastSacnInterfaceName(defaultIfaceName);
    } else {
        widgets_.sacnIfaceSelect->setCurrentIndex(
            widgets_.sacnIfaceSelectModel->GetRowForInterfaceName(
                lastSacnInterfaceName.toStdString()));
    }
    configForm->addRow(tr("sACN Interface"), widgets_.sacnIfaceSelect);
    connect(
        widgets_.sacnIfaceSelect,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &MainWindow::currentSacnIfaceChanged);

    auto *buttonsLayout = new QHBoxLayout;
    // Start button
    widgets_.startButton = new QPushButton(this);
    buttonsLayout->addWidget(widgets_.startButton);
    connect(widgets_.startButton, &QPushButton::clicked, this, &MainWindow::startApp);

    // Help button
    auto *helpButton = new QPushButton(tr("Help"), this);
    buttonsLayout->addWidget(helpButton);
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::help);

    configForm->addRow(buttonsLayout);

    // Suppress sleep checkbox
    widgets_.suppressSleep = new QCheckBox(tr("Suppress sleep"), this);
    connect(widgets_.suppressSleep, &QCheckBox::toggled, this, &MainWindow::suppressSleepChanged);
    configForm->addRow(widgets_.suppressSleep);

    sidebarLayout->addStretch();

    // QR code
    auto *qrLayout = new QHBoxLayout;
    qrLayout->setAlignment(Qt::AlignCenter);
    sidebarLayout->addLayout(qrLayout);
    widgets_.qrCode = new QrCode(this);
    qrLayout->addWidget(widgets_.qrCode);
    sidebarLayout->addStretch();

    // Log viewer
    widgets_.logViewer = new LogViewer(this);
    layout->addWidget(widgets_.logViewer);

    appStopped();
}

void MainWindow::startApp()
{
    app_->run(appOptions);
    disconnect(widgets_.startButton, &QPushButton::clicked, this, &MainWindow::startApp);
    connect(widgets_.startButton, &QPushButton::clicked, this, &MainWindow::stopApp);
    appStarted();
}

void MainWindow::stopApp()
{
    app_->stop();
    disconnect(widgets_.startButton, &QPushButton::clicked, this, &MainWindow::stopApp);
    connect(widgets_.startButton, &QPushButton::clicked, this, &MainWindow::startApp);
    appStopped();
}

void MainWindow::help()
{
    const auto helpPath = QString("file:///%1/../%2/doc/index.html")
                              .arg(qApp->applicationDirPath(), config::kWebPath);
    QDesktopServices::openUrl(QUrl(helpPath, QUrl::TolerantMode));
}

void MainWindow::suppressSleepChanged(bool suppress)
{
    auto *caffeine = Caffeine::get();
    caffeine->setActive(suppress);
}

void MainWindow::appStarted()
{
    widgets_.startButton->setText(tr("Stop"));
    widgets_.qrCode->setContents(app_->getWebUrl());
}

void MainWindow::appStopped()
{
    widgets_.startButton->setText(tr("Start"));
    widgets_.qrCode->clear();
}

void MainWindow::currentWebUiIfaceChanged(int row)
{
    const auto &iface = widgets_.webuiIfaceSelectModel->GetNetIntInfo(row);
    appOptions.backend_address = iface.addr().ToString();
}

void MainWindow::currentSacnIfaceChanged(int row)
{
    const auto &iface = widgets_.sacnIfaceSelectModel->GetNetIntInfo(row);
    appOptions.sacn_address = iface.addr().ToString();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    stopApp();
    Settings::SetMainWindowGeometry(saveGeometry());
    Settings::Sync();
    event->accept();
}

} // namespace mobilesacn
