/**
 * @file Release.h
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#ifndef MOBILESACN_MOBILESACN_UPDATER_RELEASE_H
#define MOBILESACN_MOBILESACN_UPDATER_RELEASE_H

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QUrl>

namespace mobilesacn {
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
    QString downloadFilename;
    QUrl downloadUrl;
};
} // namespace mobilesacn

Q_DECLARE_METATYPE(mobilesacn::Release)

#endif //MOBILESACN_MOBILESACN_UPDATER_RELEASE_H
