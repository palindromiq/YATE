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
#include "updater.h"
#include "globals.h"
#include "downloader.h"


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
    setStatus(UpdaterStatus::Downloaded);
    QString selfPath = QCoreApplication::applicationFilePath();
    QFile currentSelf(selfPath);
    QString newPath = selfPath + "~";
    if (QFileInfo::exists(newPath)) {
        if(!QFile::remove(newPath)) {
            setStatus(UpdaterStatus::UpdateFailed);
            emit errorOccurred("Update failed.");
            return;
        }
    }
    if (!currentSelf.rename(newPath)) {
        setStatus(UpdaterStatus::UpdateFailed);
        qDebug() << "1: " << currentSelf.errorString();
        emit errorOccurred("Update failed.");
        return;
    }

    QFile newVersion(filePath);
    if (!newVersion.rename(selfPath)) {
        setStatus(UpdaterStatus::UpdateFailed);
        qDebug() << "2: " << newVersion.errorString();
        currentSelf.rename(selfPath);
        emit errorOccurred("Update failed.");
        return;
    }
    setStatus(UpdaterStatus::PendingRestart);
    QProcess::startDetached(selfPath, {"update", newPath, QString::number(QCoreApplication::applicationPid()),latestVersion_});
    QCoreApplication::exit(0);
}

void Updater::onDownloadFailed(QString err) {
    setStatus(UpdaterStatus::UpdateFailed);
    emit errorOccurred(err);
}

void Updater::onManagerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        setStatus(UpdaterStatus::CheckFailed);
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
            setStatus(UpdaterStatus::UpToDate);
        } else {
            setStatus(UpdaterStatus::UpdateAvailable);
        }

    }
}


}
