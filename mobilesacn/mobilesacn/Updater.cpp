/**
 * @file Updater.cpp
 *
 * @author Dan Keenan
 * @date 5/3/26
 * @copyright Apache-2.0
 */

#include "Updater.h"
#include "mobilesacn_config.h"
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QDesktopServices>
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
#include <QProcess>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QVersionNumber>
#ifdef OS_WINDOWS
#include <Windows.h>
#include <msi.h>
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
#ifdef OS_WINDOWS
    // Get MSI product code (which changes each release) from upgrade code (always the same).
    // If we can find the product code, the program was installed with an MSI.
    std::wstring productCode(MAX_GUID_CHARS, L'\0');
    // https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msienumrelatedproductsw
    auto ret = MsiEnumRelatedProducts(config::kProjectMsiUpgradeCode, 0, 0, productCode.data());
    if (ret == ERROR_SUCCESS) {
        // Software was installed with an MSI.
        filenameRegexes.emplace_back(QStringLiteral("-Windows\\.msi$"));
    }

    filenameRegexes.emplace_back(QStringLiteral("-Windows\\.zip$"));
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

UpdateDialog::UpdateDialog(const Updater::Release &release, QWidget *parent) :
    QDialog(parent), release_(release), nam_(new QNetworkAccessManager(this))
{
    setWindowTitle(tr("Software Update"));
    resize(640, 480);

    // Title
    auto layout = new QVBoxLayout(this);
    auto title = new QLabel(
        tr("<big><strong>A new version of %1 is available!</strong></big>")
            .arg(qApp->applicationDisplayName()),
        this);
    layout->addWidget(title);

    // Version info
    auto versionLayout = new QHBoxLayout;
    auto currentVersion
        = new QLabel(tr("Current Version: %1").arg(qApp->applicationVersion()), this);
    versionLayout->addWidget(currentVersion);
    auto newVersion = new QLabel(tr("New Version: %1").arg(release.version), this);
    versionLayout->addWidget(newVersion);
    auto releaseDate = new QLabel(
        tr("Released: %1").arg(QLocale().toString(release.publishedAt, QLocale::ShortFormat)), this);
    versionLayout->addWidget(releaseDate);
    versionLayout->addStretch();
    layout->addLayout(versionLayout);

    // Release notes label
    auto releaseNotesLabel = new QLabel(tr("Release Notes:"), this);
    layout->addWidget(releaseNotesLabel);

    // Release notes
    auto releaseNotes = new QTextBrowser(this);
    releaseNotes->setOpenExternalLinks(true);
    releaseNotes->setHtml(release.releaseNotes);
    layout->addWidget(releaseNotes);

    // More details
    auto moreDetails = new QLabel(
        tr("<a href=\"%1\">More Details...</a>").arg(release.url.toString()));
    moreDetails->setOpenExternalLinks(true);
    layout->addWidget(moreDetails);

    // Actions
    auto buttonBox = new QDialogButtonBox(this);
    if (!release.downloadUrl.isEmpty()) {
        buttonBox->addButton(tr("Install"), QDialogButtonBox::AcceptRole);
    }
    buttonBox->addButton(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UpdateDialog::installUpdate);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &UpdateDialog::close);
    layout->addWidget(buttonBox);
}

void UpdateDialog::installUpdate()
{
    auto tempDir = new QTemporaryDir;
    if (!tempDir->isValid()) {
        SPDLOG_ERROR("Could not create download location.");
        delete tempDir;
        return;
    }

    auto download = new QFile(tempDir->filePath(release_.downloadFilename), this);
    if (!download->open(QFile::WriteOnly)) {
        SPDLOG_ERROR("Error opening file for writing.");
        delete tempDir;
        return;
    }

    // Can't autoremove as the installer needs to exist after this program closes.
    tempDir->setAutoRemove(false);

    auto *progress = new QProgressDialog(this);
    progress->setLabelText(tr("Downloading update..."));

    auto resp = nam_->get(QNetworkRequest(release_.downloadUrl));
    connect(progress, &QProgressDialog::canceled, [this, resp, tempDir]() {
        resp->abort();
        tempDir->remove();
        delete tempDir;
    });
    connect(
        resp, &QNetworkReply::downloadProgress, [progress](qint64 bytesReceived, qint64 bytesTotal) {
            progress->setMaximum(bytesTotal);
            progress->setValue(bytesReceived);
        });
    connect(resp, &QNetworkReply::readyRead, [download, resp]() {
        QByteArray buf(1024, 0);
        qint64 readCount;
        while ((readCount = resp->read(buf.data(), buf.size())) > 0) {
            download->write(buf.data(), readCount);
        }
    });
    connect(resp, &QNetworkReply::finished, [this, download, resp, tempDir]() {
        resp->deleteLater();
        download->close();
        download->deleteLater();

        if (resp->error()) {
            tempDir->remove();
            delete tempDir;
            if (resp->error() == QNetworkReply::OperationCanceledError) {
                // User cancelled.
                SPDLOG_INFO("Cancelled update.");
                return;
            }

            SPDLOG_ERROR("Error downloading update: {}", resp->errorString().toStdString());
            QMessageBox::critical(this, tr("Error downloading file"), resp->errorString());
            return;
        }

        QUrl url;
        url.setScheme("file");
        url.setPath(download->fileName());
        if (QDesktopServices::openUrl(url)) {
            qApp->quit();
        } else {
            QMessageBox::critical(
                this, tr("Error opening file"), tr("The downloaded file could not be opened."));
        }
        delete tempDir;
    });
}

} // namespace mobilesacn
