/**
 * @file Updater.cpp
 *
 * @author Dan Keenan
 * @date 5/3/26
 * @copyright GPL-3.0
 */

#include "Updater.h"
#include "mobilesacn_config.h"
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QVersionNumber>
#include <QtConcurrentRun>

#define UPDATER_CHECK_VERSION "0.0.0"

#ifndef UPDATER_CHECK_VERSION
#define UPDATER_CHECK_VERSION mobilesacn::config::kProjectVersion
#endif

namespace mobilesacn {

static const auto kGitHubRepo = QStringLiteral("danielskeenan/mobile_sacn");

/**
 * Make a GitHub API request.
 * @param path
 * @param params
 * @return
 */
QNetworkRequest ghReq(const QString &path, const QUrlQuery &params = {})
{
    QUrl reqUrl(QStringLiteral("https://api.github.com"));
    reqUrl.setPath(path);
    reqUrl.setQuery(params);
    QNetworkRequest req(reqUrl);
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setRawHeader("X-GitHub-Api-Version", "2026-03-10");
    return req;
}

/**
 * Find the correct package for this system.
 * @return
 */
QString preferredPackage(const QStringList &filenames)
{
    std::vector<QRegularExpression> filenameRegexes;

#ifdef OS_LINUX
    // Which package manager does this system use?
    const auto usesDeb = []() {
        // https://manpages.debian.org/stretch/dpkg/dpkg-query.1.en.html
        QProcess dpkgQuery;
        dpkgQuery.start("dpkg-query", {"--search", qApp->applicationFilePath()});
        dpkgQuery.waitForFinished();
        if (dpkgQuery.exitCode() == 0) {
            QRegularExpression re(QStringLiteral(".+: %1").arg(
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
#endif

    // Find first matching filename.
    for (const auto &re : filenameRegexes) {
        for (const auto &filename : filenames) {
            if (re.match(filename).hasMatch()) {
                return filename;
            }
        }
    }

    return {};
}

Updater::Updater(QObject *parent) : QObject(parent), nam_(new QNetworkAccessManager(this)) {}

void Updater::checkForUpdate()
{
    SPDLOG_INFO("Checking for software updates...");
    auto req = ghReq(QStringLiteral("/repos/%1/releases/latest").arg(kGitHubRepo));
    auto resp = nam_->get(req);
    connect(resp, &QNetworkReply::finished, [this, resp]() {
        resp->deleteLater();
        if (resp->error()) {
            SPDLOG_WARN("Error checking for GitHub releases: {}", resp->errorString().toStdString());
            return;
        }
        QJsonParseError err;
        const auto doc = QJsonDocument::fromJson(resp->readAll(), &err);
        if (!doc.isObject() || err.error != QJsonParseError::NoError) {
            SPDLOG_WARN("Bad JSON response from server: {}", err.errorString().toStdString());
            return;
        }

        // Determine release version number.
        const auto &tagName = doc["tag_name"];
        if (!tagName.isString()) {
            SPDLOG_WARN("Missing tag_name from response.");
            return;
        }
        const auto releaseVersion = QVersionNumber::fromString(tagName.toString().mid(1));
        if (QVersionNumber::fromString(UPDATER_CHECK_VERSION) >= releaseVersion) {
            SPDLOG_INFO("No updates available.");
            return;
        }

        Release release;

        // Name
        const auto &name = doc["name"];
        if (!name.isString()) {
            SPDLOG_WARN("Missing name from response.");
            return;
        }
        release.name = name.toString();

        // Published At
        const auto &publishedAt = doc["published_at"];
        if (!publishedAt.isString()) {
            SPDLOG_WARN("Missing published_at from response.");
            return;
        }
        release.publishedAt = QDateTime::fromString(publishedAt.toString(), Qt::ISODate);

        // URL
        const auto &url = doc["html_url"];
        if (!url.isString()) {
            SPDLOG_WARN("Missing html_url from response.");
            return;
        }
        release.url = url.toString();

        // Download file
        const auto &assets = doc["assets"];
        if (!assets.isArray()) {
            SPDLOG_WARN("Missing assets from response.");
            return;
        }
        QHash<QString, QString> assetUrls;
        for (const auto &asset : assets.toArray()) {
            if (!asset.isObject()) {
                SPDLOG_WARN("Asset is malformed.");
                return;
            }
            const auto &assetName = asset[QLatin1StringView("name")];
            if (!assetName.isString()) {
                SPDLOG_WARN("Missing Asset name");
                return;
            }
            const auto &downloadUrl = asset[QLatin1StringView("browser_download_url")];
            if (!downloadUrl.isString()) {
                SPDLOG_WARN("Missing Asset browser_download_url");
                return;
            }
            assetUrls.emplace(assetName.toString(), downloadUrl.toString());
        }
        const auto filename = preferredPackage(assetUrls.keys());
        if (!filename.isEmpty()) {
            release.downloadUrl = assetUrls[filename];
        }

        // Release Notes
        const auto body = doc["body"];
        if (!body.isString()) {
            SPDLOG_WARN("Missing body from response.");
            return;
        }
        // Ask GitHub to format this Markdown so links resolve properly.
        auto mdReq = ghReq("/markdown");
        mdReq.setRawHeader("Accept", "text/html");
        mdReq.setRawHeader("Content-Type", "application/json");
        QJsonObject mdReqBody{
            {"text", body},
            {"mode", "gfm"},
            {"context", kGitHubRepo},
        };
        auto mdResp = nam_->post(mdReq, QJsonDocument(mdReqBody).toJson());
        connect(mdResp, &QNetworkReply::finished, [this, mdResp, release]() mutable {
            if (mdResp->error()) {
                SPDLOG_WARN(
                    "Error asking GitHub to format Markdown: {}",
                    mdResp->errorString().toStdString());
                return;
            }
            release.releaseNotes = QString::fromUtf8(mdResp->readAll());
            Q_EMIT(updateAvailable(release));
        });
    });
}

} // namespace mobilesacn
