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
#include "Updater.h"
#include "mobilesacn/libmobilesacn/Caffeine.h"
#include "mobilesacn_config.h"
#include "ui_MainWindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>

namespace mobilesacn {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui_(new Ui::MainWindow), app_(new Application(this)),
    updater_(new Updater(this)), netIntModel_(new NetIntListModel(this)),
    clientModel_(new ClientTableModel(app_, this))
{
    ui_->setupUi(this);
    if (!restoreGeometry(Settings::getMainWindowGeometry())) {
        Settings::setMainWindowGeometry(saveGeometry());
    }

    // Check for updates.
    connect(updater_, &Updater::updateAvailable, this, &MainWindow::updateAvailable);
    updater_->checkForUpdate();

    // sACN Net Int
    ui_->cmbSacnIface->setModel(netIntModel_);
    setNetIntComboBox(
        ui_->cmbSacnIface, &Settings::getLastSacnInterfaceName, &Settings::setLastSacnInterfaceName);
    on_cmbSacnIface_currentIndexChanged(ui_->cmbSacnIface->currentIndex());

    // Web Net Int
    ui_->cmbWebUiIface->setModel(netIntModel_);
    setNetIntComboBox(
        ui_->cmbWebUiIface,
        &Settings::getLastWebUiInterfaceName,
        &Settings::setLastWebUiInterfaceName);
    on_cmbWebUiIface_currentIndexChanged(ui_->cmbWebUiIface->currentIndex());

    // Lifecycle
    connect(app_, &Application::started, this, &MainWindow::appStarted);
    connect(app_, &Application::stopped, this, &MainWindow::appStopped);

    // Client table
    ui_->tblClients->setModel(clientModel_);

    appStopped();
}

void MainWindow::setNetIntComboBox(
    QComboBox *cmb,
    const std::function<QString()> &netIntNameGetter,
    const std::function<void(const QString &)> &netIntNameSetter)
{
    const QString &ifaceName = netIntNameGetter();
    if (ifaceName.isEmpty()) {
        const auto defaultNetIntRow = netIntModel_->GetDefaultRow();
        cmb->setCurrentIndex(defaultNetIntRow);
        const auto defaultNetIntName
            = netIntModel_->data(netIntModel_->index(defaultNetIntRow, 0, {}), Qt::DisplayRole)
                  .toString();
        netIntNameSetter(defaultNetIntName);
    } else {
        cmb->setCurrentIndex(netIntModel_->GetRowForInterfaceName(ifaceName.toStdString()));
    }
}

void MainWindow::on_btnStart_clicked()
{
    if (app_->isRunning()) {
        app_->stop();
    } else {
        app_->start(appOptions_);
    }
}

void MainWindow::on_btnHelp_clicked()
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
    ui_->btnStart->setText(tr("Stop"));
    ui_->lblUrl->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(app_->getWebUrl()));
    // widgets_.qrCode->setContents(app_->getWebUrl());

    // Update settings.
    const auto webUiNetIntRow = ui_->cmbWebUiIface->currentIndex();
    const auto webUiNetInt = netIntModel_->GetNetIntInfo(webUiNetIntRow);
    Settings::setLastWebUiInterfaceName(QString::fromStdString(webUiNetInt.friendly_name()));
    const auto sacnNetIntRow = ui_->cmbSacnIface->currentIndex();
    const auto sacnNetInt = netIntModel_->GetNetIntInfo(sacnNetIntRow);
    Settings::setLastSacnInterfaceName(QString::fromStdString(sacnNetInt.friendly_name()));
}

void MainWindow::appStopped()
{
    ui_->btnStart->setText(tr("Start"));
    ui_->lblUrl->clear();
    // widgets_.qrCode->clear();
}

void MainWindow::on_cmbWebUiIface_currentIndexChanged(int row)
{
    const auto &iface = netIntModel_->GetNetIntInfo(row);
    appOptions_.backend_address = iface.addr().ToString();
}

void MainWindow::on_cmbSacnIface_currentIndexChanged(int row)
{
    const auto &iface = netIntModel_->GetNetIntInfo(row);
    appOptions_.sacn_address = iface.addr().ToString();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    app_->stop();
    Settings::setMainWindowGeometry(saveGeometry());
    Settings::sync();
    event->accept();
}

void MainWindow::updateAvailable(const Updater::Release &release)
{
    auto *dialog = new UpdateDialog(release, this);
    connect(dialog, &UpdateDialog::finished, dialog, &UpdateDialog::deleteLater);
    dialog->setModal(true);
    dialog->show();
}

} // namespace mobilesacn
