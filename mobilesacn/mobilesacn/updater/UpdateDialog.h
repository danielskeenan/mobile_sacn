/**
 * @file UpdateDialog.h
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_MOBILESACN_UPDATER_UPDATEDIALOG_H
#define MOBILESACN_MOBILESACN_UPDATER_UPDATEDIALOG_H

#include "Release.h"
#include <QDialog>

namespace mobilesacn {

/**
 * Present an available update.
 */
class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(const Release &release, QWidget *parent = nullptr);

private:
    Release release_;

private Q_SLOTS:
    void installUpdate();
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_UPDATER_UPDATEDIALOG_H
