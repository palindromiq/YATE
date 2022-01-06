#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QSslError>
#include <QMap>

class QNetworkReply;
class QNetworkAccessManager;

namespace Yate {

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);
    QString saveToDisk(QIODevice *reply, bool &result);

signals:
    void onDownloadFinished(QString, QString);
    void onDownloadFailed(QString);

private slots:
    void onManagerFinished(QNetworkReply *);
#ifndef QT_NO_SSL
    void onSslErrors(QList<QSslError>);
#endif
public slots:
    void download(QString urlString, QString expectedHash);


private:
    QString getFileHash(QString filePath);
    QNetworkAccessManager *manager_;
    QString saveError_;
    QAtomicInt isDownloading_;
    QString expectedHash_;

};

}

#endif // DOWNLOADER_H
