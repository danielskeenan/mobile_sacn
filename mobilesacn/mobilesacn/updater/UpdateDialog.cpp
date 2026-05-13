/**
 * @file UpdateDialog.cpp
 *
 * @author Dan Keenan
 * @date 5/13/2026
 * @copyright GPL-3.0
 */

#include "UpdateDialog.h"
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace mobilesacn {

UpdateDialog::UpdateDialog(const Release &release, QWidget *parent) :
    QDialog(parent), release_(release)
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

    auto nam = new QNetworkAccessManager(this);
    auto resp = nam->get(QNetworkRequest(release_.downloadUrl));
    connect(progress, &QProgressDialog::canceled, [resp, tempDir, nam]() {
        resp->abort();
        tempDir->remove();
        nam->deleteLater();
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
    connect(resp, &QNetworkReply::finished, [this, download, resp, tempDir, nam]() {
        resp->deleteLater();
        nam->deleteLater();
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
