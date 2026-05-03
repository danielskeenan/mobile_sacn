/**
 * @file Updater.h
 *
 * @author Dan Keenan
 * @date 5/3/26
 * @copyright GPL-3.0
 */

#ifndef MOBILESACN_MOBILESACN_UPDATER_H
#define MOBILESACN_MOBILESACN_UPDATER_H

#include <QDateTime>
#include <QDialog>
#include <QFuture>
#include <QNetworkAccessManager>
#include <QUrl>

namespace mobilesacn {

/**
 * Find updates from GitHub releases.
 */
class Updater : public QObject
{
    Q_OBJECT
public:
    /**
     * Hold information about a release.
     */
    struct Release
    {
        QString name;
        QString version;
        QDateTime publishedAt;
        QUrl url;
        /** HTML release notes */
        QString releaseNotes;
        QUrl downloadUrl;
    };

    explicit Updater(QObject *parent = nullptr);

    void checkForUpdate();

Q_SIGNALS:
    void updateAvailable(const Release &release);

private:
    QNetworkAccessManager *nam_;
};

/**
 * Present an available update.
 */
class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(const Updater::Release &release, QWidget *parent = nullptr);
};

} // namespace mobilesacn

#endif //MOBILESACN_MOBILESACN_UPDATER_H
