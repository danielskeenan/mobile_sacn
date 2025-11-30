/**
 * @file MainWindow.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
#define MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_

#include "LogViewer.h"
#include "QrCode.h"
#include "mobilesacn/libmobilesacn/Application.h"
#include <QCheckBox>
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

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
    struct Widgets
    {
        QComboBox *webuiIfaceSelect = nullptr;
        NetIntListModel *webuiIfaceSelectModel = nullptr;
        QComboBox *sacnIfaceSelect = nullptr;
        NetIntListModel *sacnIfaceSelectModel = nullptr;
        QPushButton *startButton = nullptr;
        QrCode *qrCode = nullptr;
        QCheckBox *suppressSleep = nullptr;
        LogViewer *logViewer = nullptr;
    };
    Widgets widgets_;
    Application *app_ = nullptr;
    Application::Options appOptions;

    void initUi();

protected Q_SLOTS:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void startApp();
    void stopApp();
    void help();
    void suppressSleepChanged(bool suppress);
    void appStarted();
    void appStopped();
    void currentWebUiIfaceChanged(int row);
    void currentSacnIfaceChanged(int row);
};
} // namespace mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
