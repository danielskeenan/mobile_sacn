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
    QComboBox *webui_iface_select = nullptr;
    NetIntListModel *webui_iface_select_model = nullptr;
    QComboBox *sacn_iface_select = nullptr;
    NetIntListModel *sacn_iface_select_model = nullptr;
    QPushButton *start_button = nullptr;
    QrCode *qr_code = nullptr;
    LogViewer *log_viewer = nullptr;
  };
  Widgets widgets_;
  Application app_;
  Application::Options app_options_;

  void InitUi();

 protected Q_SLOTS:
  void closeEvent(QCloseEvent *event) override;

 private Q_SLOTS:
  void SStartApp();
  void SStopApp();
  void SAppStarted();
  void SAppStopped();
  void SCurrentWebUiIfaceChanged(int row);
  void SCurrentSacnIfaceChanged(int row);
};

} // mobilesacn

#endif //MOBILE_SACN_SRC_MOBILESACN_MAINWINDOW_H_
