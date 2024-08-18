/**
 * @file MainWindow.h
 *
 * @author Dan Keenan
 * @date 4/9/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
#define MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include "libmobilesacn/Application.h"
#include "LogViewer.h"
#include "QrCode.h"

namespace mobilesacn {
class NetIntListModel;

/**
 * Main Window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private:
    struct Widgets {
      QComboBox *webuiIfaceSelect = nullptr;
      NetIntListModel *webuiIfaceSelectModel = nullptr;
      QComboBox *sacnIfaceSelect = nullptr;
      NetIntListModel *sacnIfaceSelectModel = nullptr;
      QPushButton *startButton = nullptr;
      QrCode *qrCode = nullptr;
      LogViewer *logViewer = nullptr;
    };
    Widgets widgets_;
    Application app_;
    Application::Options appOptions;

    void initUi();

  protected Q_SLOTS:
    void closeEvent(QCloseEvent *event) override;

  private Q_SLOTS:
    void startApp();
    void stopApp();
    void help();
    void appStarted();
    void appStopped();
    void currentWebUiIfaceChanged(int row);
    void currentSacnIfaceChanged(int row);
};
} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
