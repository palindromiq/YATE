#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class QThread;
class QNetworkAccessManager;
class QNetworkReply;

namespace Yate {

class Downloader;

enum class UpdaterStatus {
    Unknown,
    Checking,
    CheckFailed,
    UpdateFailed,
    UpdateAvailable,
    Downloading,
    Downloaded,
    Installing,
    PendingRestart,
    UpToDate,
};

class Updater : public QObject
{
    Q_OBJECT
public:

    static Updater *getInstance(QObject *parent = nullptr);
    Updater(const Updater &) = delete;
    void operator=(const Updater &) = delete;
    void checkUpdate();
    QString getVersion() const;

    UpdaterStatus status() const;

    static QVector<int> parseVersion(QString ver);



    const QString &latestVersion() const;

public slots:
    void checkForUpdate();
    void startUpdate();
private slots:
    void onDownloadFinished(QString, QString);
    void onDownloadFailed(QString);
    void onManagerFinished(QNetworkReply *);

signals:
    void updateAvailable();
    void updateStatusUpdate(QString status);
    void errorOccurred(QString err);
    void onBusyUpdate(bool);


private:
    explicit Updater(QObject *parent = nullptr);
    void setStatus(UpdaterStatus newStatus);
    void downloadUpdate();
    void setLatestVersion(const QString &newLatestVersion);

    static Updater *instance_;
    static QThread *thread_;
    QNetworkAccessManager *manager_;
    UpdaterStatus status_;
    Downloader *downloader_;
    QString downloadHash_;
    QString downloadUrl_;
    QString latestVersion_;
};

}

#endif // UPDATER_H
