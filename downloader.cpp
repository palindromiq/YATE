#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QTemporaryFile>
#include "downloader.h"

namespace Yate {

Downloader::Downloader(QObject *parent)
    : QObject{parent}, manager_(new QNetworkAccessManager)
{
    isDownloading_.storeRelaxed(0);
    expectedHash_ = "";
    connect(manager_, &QNetworkAccessManager::finished, this, &Downloader::onManagerFinished);
}

void Downloader::onManagerFinished(QNetworkReply *reply)
{
    qDebug() << "Download finished.";
    isDownloading_.storeRelaxed(0);
    if (reply->error()) {
        qCritical() << "Download failed with network error " << reply->errorString();
        emit onDownloadFailed("Update download failed: " + reply->errorString());
    } else {
        bool ok;
        QString filename = saveToDisk(reply, ok);

        if (ok) {
            qDebug() << "Saved download file to desk.";
            QString hash = getFileHash(filename);
            if (expectedHash_.size() && expectedHash_ != hash) {
                qCritical() << "Hash mismatch for downloaded file";
                emit onDownloadFailed("Update download failed: hash validation failed");
            } else {
                qDebug() << "Hash is ok for downloaded file";
                emit onDownloadFinished(filename, hash);
            }


        } else {
            qCritical() << "Failed to save download file: " << saveError_;
            emit onDownloadFailed(saveError_);
        }
    }
    expectedHash_ = "";

}

QString Downloader::saveToDisk(QIODevice *reply, bool &result)
{
    qDebug() << "Saving download file to disk.";
    QTemporaryFile file(QDir::tempPath() + QDir::separator() + "XXXXXX_YATE.zip");
    if (!file.open()) {
        saveError_ = file.errorString();
        result = false;
        return "";
    }
    file.setAutoRemove(false);

    file.write(reply->readAll());
    file.close();
    saveError_ = "";

    result = true;
    return file.fileName();
}

#ifndef QT_NO_SSL
void Downloader::onSslErrors(QList<QSslError> errs)
{
    for(auto &e: errs) {
        qCritical() << e.errorString();
    }
}
#endif

void Downloader::download(QString urlString, QString expectedHash)
{
    if (isDownloading_.loadRelaxed() != 1) {
        qDebug() << "Downloading " << urlString;
        QUrl url = QUrl::fromEncoded(urlString.toLocal8Bit());
        isDownloading_.storeRelaxed(1);
        expectedHash_ = expectedHash;
        QNetworkRequest request(url);
        QNetworkReply *reply = manager_->get(request);
#ifndef QT_NO_SSL
        connect(reply, &QNetworkReply::sslErrors, this, &Downloader::onSslErrors);
#endif
    } else {
        qWarning() << "Already downloading..";
    }

}

QString Downloader::getFileHash(QString filePath)
{
    QFile f(filePath);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&f)) {
            return hash.result().toHex();
        }
    }
    return "";
}

}


