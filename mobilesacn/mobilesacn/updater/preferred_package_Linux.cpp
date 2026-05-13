/**
 * @file preferred_package_Linux.cpp
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright Apache-2.0
 */

#include "preferred_package.h"
#include <QApplication>
#include <QProcess>
#include <QRegularExpression>

namespace mobilesacn::detail {
QStringList preferredPackageRegexes()
{
    QStringList filenameRegexes;

    // Which package manager does this system use?
    const auto usesDeb = []() {
        // https://manpages.debian.org/stretch/dpkg/dpkg-query.1.en.html
        QProcess dpkgQuery;
        dpkgQuery.start("dpkg-query", {"--search", qApp->applicationFilePath()});
        dpkgQuery.waitForFinished();
        if (dpkgQuery.exitCode() == 0) {
            const QRegularExpression re(QStringLiteral(".+: %1").arg(
                QRegularExpression::escape(qApp->applicationFilePath())));
            return re.match(dpkgQuery.readAllStandardOutput().trimmed()).hasMatch();
        }
        return false;
    }();
    if (usesDeb) {
        filenameRegexes.emplace_back(
            QStringLiteral("_%1\\.deb$").arg(QSysInfo::currentCpuArchitecture()));
    }

    const auto usesRpm = []() {
        // https://www.man7.org/linux/man-pages/man8/rpm.8.html
        QProcess rpm;
        rpm.start("rpm", {"--query", "--path", qApp->applicationFilePath()});
        rpm.waitForFinished();
        if (rpm.exitCode() == 0) {
            return !rpm.readAllStandardOutput().trimmed().isEmpty();
        }
        return false;
    }();
    if (usesRpm) {
        filenameRegexes.emplace_back(QStringLiteral("-Linux\\.rpm$"));
    }

    filenameRegexes.emplace_back(QStringLiteral("-Linux\\.tar\\.gz$"));

    return filenameRegexes;
}
} // namespace mobilesacn::detail
