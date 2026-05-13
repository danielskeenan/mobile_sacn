/**
 * @file Updater.h
 *
 * @author Dan Keenan
 * @date 5/3/26
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_MOBILESACN_UPDATER_UPDATER_H
#define MOBILESACN_MOBILESACN_UPDATER_UPDATER_H

#include "Release.h"
#include <QDateTime>
#include <QDialog>
#include <QFuture>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QUrl>

namespace mobilesacn {

/**
 * Find updates from GitHub releases.
 */
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = nullptr);

    void checkForUpdate();

Q_SIGNALS:
    void updateAvailable(const Release &release);

private:
    QNetworkAccessManager *nam_;
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_UPDATER_UPDATER_H
