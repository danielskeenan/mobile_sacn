/**
 * @file Updater.cpp
 *
 * @author Dan Keenan
 * @date 5/3/26
 * @copyright Apache-2.0
 */

#include "Updater.h"
#include "mobilesacn_config.h"
#include "preferred_package.h"
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QMessageBox>
#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextBrowser>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QVersionNumber>

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

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent), nam_(new QNetworkAccessManager(this))
{}

void UpdateChecker::checkForUpdates()
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
        if (QVersionNumber::fromString(qApp->applicationVersion()) >= releaseVersion) {
            SPDLOG_INFO("No updates available.");
            return;
        }

        Release release;
        release.version = releaseVersion.toString();
        SPDLOG_INFO("Update is available to {}", release.version.toStdString());

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
            release.downloadFilename = filename;
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

Updater::Updater(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Release>();

    auto *updateChecker = new UpdateChecker;
    updateChecker->moveToThread(&updateCheckerThread_);
    connect(&updateCheckerThread_, &QThread::finished, updateChecker, &UpdateChecker::deleteLater);
    connect(this, &Updater::doCheckForUpdates, updateChecker, &UpdateChecker::checkForUpdates);
    connect(updateChecker, &UpdateChecker::updateAvailable, this, &Updater::updateAvailable);
    updateCheckerThread_.start();
}

Updater::~Updater()
{
    updateCheckerThread_.quit();
    updateCheckerThread_.wait();
}

void Updater::checkForUpdates()
{
    Q_EMIT(doCheckForUpdates());
}

} // namespace mobilesacn
