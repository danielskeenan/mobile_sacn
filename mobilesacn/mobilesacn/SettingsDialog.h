/**
 * @file SettingsDialog.h
 *
 * @author Dan Keenan
 * @date 5/16/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_MOBILESACN_SETTINGSDIALOG_H
#define MOBILESACN_MOBILESACN_SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

namespace mobilesacn {

/**
 * Settings Dialog.
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void accept() override;

private:
    Ui::SettingsDialog *ui_;
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_SETTINGSDIALOG_H
