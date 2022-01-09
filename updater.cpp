#include <QApplication>
#include <QThread>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include "updater.h"
#include "globals.h"
#include "downloader.h"
#include "zipmanager.h"

namespace Yate {

Updater * Updater::instance_ = nullptr;
QThread * Updater::thread_ = nullptr;


Updater *Updater::getInstance(QObject *)
{
    if (instance_ == nullptr) {
        instance_ = new Updater;
        thread_ = new QThread;
        instance_->moveToThread(thread_);
        thread_->start();
    }
    return instance_;
}

Updater::Updater(QObject *parent)
    : QObject{parent}, manager_(new QNetworkAccessManager(this)), downloader_(new Downloader(this))
{
    setStatus(UpdaterStatus::Unknown);
    connect(manager_, &QNetworkAccessManager::finished, this, &Updater::onManagerFinished);
    connect(downloader_, &Downloader::onDownloadFinished, this, &Updater::onDownloadFinished);
    connect(downloader_, &Downloader::onDownloadFailed, this, &Updater::onDownloadFailed);
}

void Updater::setStatus(UpdaterStatus newStatus)
{
    status_ = newStatus;

    switch (status_) {
    case UpdaterStatus::Unknown: {
        emit onBusyUpdate(false);
        emit updateStatusUpdate("N/A");
        break;
    }
    case UpdaterStatus::CheckFailed: {
        emit updateStatusUpdate("Check Failed");
        emit onBusyUpdate(false);
        break;
    }
    case UpdaterStatus::Checking: {
        emit updateStatusUpdate("Checking");
        emit onBusyUpdate(true);
        break;
    }
    case UpdaterStatus::UpdateFailed: {
        emit updateStatusUpdate("Update Failed");
        emit onBusyUpdate(false);
        break;
    }
    case UpdaterStatus::UpdateAvailable: {
        emit onBusyUpdate(false);
        emit updateStatusUpdate("Update Available");
        emit updateAvailable();
        break;
    }
    case UpdaterStatus::Downloading: {
        emit onBusyUpdate(true);
        emit updateStatusUpdate("Downloading..");
        break;
    }
    case UpdaterStatus::Downloaded: {
        emit onBusyUpdate(true);
        emit updateStatusUpdate("Downloaded");
        break;
    }
    case UpdaterStatus::Installing: {
        emit onBusyUpdate(true);
        emit updateStatusUpdate("Installing...");
        break;
    }
    case UpdaterStatus::PendingRestart: {
        emit onBusyUpdate(false);
        emit updateStatusUpdate("Pending Restart");
        break;
    }
    case UpdaterStatus::UpToDate: {
        emit onBusyUpdate(false);
        emit updateStatusUpdate("Up-to-date");
        break;
    }
    }

}

void Updater::downloadUpdate()
{
    qDebug() << "Downloading update.";
    setStatus(UpdaterStatus::Downloading);
    downloader_->download(downloadUrl_, downloadHash_);
}

const QString &Updater::latestVersion() const
{
    return latestVersion_;
}

void Updater::setLatestVersion(const QString &newLatestVersion)
{
    latestVersion_ = newLatestVersion;
}

UpdaterStatus Updater::status() const
{
    return status_;
}

QVector<int> Updater::parseVersion(QString ver)
{
    QVector<int> parts;
    auto verDiv = ver.split(".");
    while (verDiv.size() >= 4) {
        verDiv.pop_back();
    }
    for(auto &p: verDiv) {
        parts.push_back(p.toInt());
    }
    return parts;
}


QString Updater::getVersion() const
{
    QString ver = QCoreApplication::applicationVersion();
    if (ver.count(".") == 3) {
        auto verDiv = ver.split(".");
        verDiv.pop_back();
        ver = verDiv.join(".");
    }
    return ver;
}

void Updater::checkForUpdate()
{
    qDebug() << "Checking for update.";
    setStatus(UpdaterStatus::Checking);
    QNetworkRequest request(QUrl(SETTINGS_URL_API_CHECK_VERSION));
    manager_->get(request);
}

void Updater::startUpdate()
{
    if (status() != UpdaterStatus::UpdateAvailable) {
        return;
    }
    downloadUpdate();
}



void Updater::onDownloadFinished(QString filePath, QString) {
    qDebug() << "Downloaded update.";
    setStatus(UpdaterStatus::Downloaded);

    QString selfPath = QCoreApplication::applicationFilePath();
    QString selfDir = QCoreApplication::applicationDirPath();
    QTemporaryDir extractionDir;
    QStringList toRmList;

    if (!extractionDir.isValid()) {
        setStatus(UpdaterStatus::UpdateFailed);
        qDebug() << "Update failed, failed to create extraction directory.";
        emit errorOccurred("Update failed.");
        return;
    }
    extractionDir.setAutoRemove(false);
    ZipManager zip;
    if (!zip.unzip(filePath, extractionDir.path())) {
        setStatus(UpdaterStatus::UpdateFailed);
        qDebug() << "Update failed, failed to unzip content.";
        emit errorOccurred("Update failed.");
        return;
    }
    QString newExec;
    for(auto &f: QDir(extractionDir.path()).entryList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot)) {
        QFile extractedFile(extractionDir.path() + QDir::separator() + f);
        if (f.endsWith(".exe")) {
            newExec = extractedFile.fileName();
        }
        QString existingFilePath = selfDir + QDir::separator() + f;
        QString tempPath = existingFilePath + "~";
        if (QFileInfo::exists(tempPath)) {
            if(!QFile::remove(tempPath)) {
                setStatus(UpdaterStatus::UpdateFailed);
                qDebug() << "Update failed, failed to remove existing temp path.";
                emit errorOccurred("Update failed.");
                return;
            }
        }
        if (QFileInfo::exists(existingFilePath)) {
            QFile existingFile(existingFilePath);
            if (!existingFile.rename(tempPath)) {
                setStatus(UpdaterStatus::UpdateFailed);

                qDebug() << "Update failed, failed to replace existing file.";
                emit errorOccurred("[1] Update failed, you may need to redownload the tool from " + SETTINGS_WEBSITE_HTTPS);
                return;
            }
            toRmList.push_back(tempPath);
        }
        if (!extractedFile.rename(existingFilePath)) {
            setStatus(UpdaterStatus::UpdateFailed);
            qDebug() << "Update failed, failed to move extracted file.";
            emit errorOccurred("[2] Update failed, you may need to redownload the tool from " + SETTINGS_WEBSITE_HTTPS);
            return;
        }
    }
    QStringList argList({"update", QString::number(QCoreApplication::applicationPid()),latestVersion_});
    for(auto &rm: toRmList) {
        argList.push_back(rm);
    }
    setStatus(UpdaterStatus::PendingRestart);
    qDebug() << "Update installed, pending restart.";
    QProcess::startDetached(selfPath, argList);
    QCoreApplication::exit(0);
}

void Updater::onDownloadFailed(QString err) {
    qDebug() << "Update download failed.";
    setStatus(UpdaterStatus::UpdateFailed);
    emit errorOccurred(err);
}

void Updater::onManagerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        setStatus(UpdaterStatus::CheckFailed);
        qDebug() << "Update check failed, network error.";
        emit errorOccurred("Update check failed: " + reply->errorString());
    } else {
        QString data = QString(reply->readAll());
        QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        setLatestVersion(jsonObject.value("latestStableVersion").toString());
        downloadUrl_ = jsonObject.value("download").toString();
        downloadHash_ = jsonObject.value("md5").toString();
        auto latestVerParts = parseVersion(latestVersion());
        auto currentVerParts = parseVersion( QCoreApplication::applicationVersion());
        bool isUpToDate = true;
        for(int i = 0; i < latestVerParts.size(); i++) {
            if(latestVerParts[i] > currentVerParts[i]) {
                isUpToDate = false;
                break;
            } else if (currentVerParts[i] > latestVerParts[i]) {
                break;
            }
        }
        if (isUpToDate) {
            qDebug() << "Current version is up-to-date.";
            setStatus(UpdaterStatus::UpToDate);
        } else {
            qDebug() << "Update available.";
            setStatus(UpdaterStatus::UpdateAvailable);
        }

    }
}


}
