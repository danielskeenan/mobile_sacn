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
#include <QNetworkAccessManager>
#include <QThread>

namespace mobilesacn {

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);

public Q_SLOTS:
    void checkForUpdates();

Q_SIGNALS:
    void updateAvailable(const Release &release);

private:
    QNetworkAccessManager *nam_;
};

/**
 * Find updates from GitHub releases.
 */
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = nullptr);
    ~Updater();

public Q_SLOTS:
    void checkForUpdates();

Q_SIGNALS:
    /** @internal Runs UpdateChecker::checkForUpdates from another thread. */
    void doCheckForUpdates();
    void updateAvailable(const Release &release);

private:
    QThread updateCheckerThread_;
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_UPDATER_UPDATER_H
