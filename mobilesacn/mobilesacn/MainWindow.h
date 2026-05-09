/**
 * @file MainWindow.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
#define MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_

#include "ClientTableModel.h"
#include "QrCode.h"
#include "Updater.h"
#include "mobilesacn/libmobilesacn/Application.h"
#include <QComboBox>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace mobilesacn {
class NetIntListModel;

/**
 * Main Window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    Ui::MainWindow *ui_;
    Application *app_ = nullptr;
    Application::Options appOptions_;
    Updater *updater_ = nullptr;
    NetIntListModel *netIntModel_;
    ClientTableModel *clientModel_;

    void setNetIntComboBox(
        QComboBox *cmb,
        const std::function<QString()> &netIntNameGetter,
        const std::function<void(const QString &)> &netIntNameSetter);

protected Q_SLOTS:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void on_btnStart_clicked();
    void on_btnHelp_clicked();
    void suppressSleepChanged(bool suppress);
    void appStarted();
    void appStopped();
    void on_cmbWebUiIface_currentIndexChanged(int row);
    void on_cmbSacnIface_currentIndexChanged(int row);
    void updateAvailable(const Updater::Release &release);
};
} // namespace mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
