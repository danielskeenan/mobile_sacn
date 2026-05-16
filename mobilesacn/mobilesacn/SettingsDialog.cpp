/**
 * @file SettingsDialog.cpp
 *
 * @author Dan Keenan
 * @date 5/16/26
 * @copyright GPL-3.0
 */

#include "SettingsDialog.h"
#include "mobilesacn/libmobilesacn/Settings.h"
#include "ui_SettingsDialog.h"

namespace mobilesacn {

void selectFromSetting(QComboBox *cmb, const QString &settingValue)
{
    for (int ix = 0; ix < cmb->count(); ++ix) {
        const auto &data = cmb->itemData(ix).toString();
        if (data == settingValue) {
            cmb->setCurrentIndex(ix);
            return;
        }
    }

    // Clear selection.
    cmb->setCurrentIndex(-1);
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::SettingsDialog)
{
    ui_->setupUi(this);

    // Check for Updates
    ui_->chkUpdater->setChecked(Settings::getCheckForUpdates());

    // Color Scheme
    // See ClientSettings.cpp
    ui_->cmbColorScheme->addItem(tr("Automatic"), QStringLiteral(""));
    ui_->cmbColorScheme->addItem(tr("Light"), QStringLiteral("light"));
    ui_->cmbColorScheme->addItem(tr("Dark"), QStringLiteral("dark"));
    selectFromSetting(ui_->cmbColorScheme, Settings::getPreferredColorScheme());

    // Level Display
    // See ClientSettings.cpp
    ui_->cmbLevelDisplay->addItem(tr("Decimal (255)"), QStringLiteral("decimal"));
    ui_->cmbLevelDisplay->addItem(tr("Hex (FF)"), QStringLiteral("hex"));
    ui_->cmbLevelDisplay->addItem(tr("Percent (100%)"), QStringLiteral("percent"));
    selectFromSetting(ui_->cmbLevelDisplay, Settings::getLevelDisplayMode());
}

void SettingsDialog::accept()
{
    // Check for Updates
    Settings::setCheckForUpdates(ui_->chkUpdater->isChecked());

    // Color Scheme
    Settings::setPreferredColorScheme(ui_->cmbColorScheme->currentData().toString());

    // Level Display
    Settings::setLevelDisplayMode(ui_->cmbLevelDisplay->currentData().toString());

    QDialog::accept();
}

} // namespace mobilesacn
