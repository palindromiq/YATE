#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QAtomicInteger>

namespace Yate {
class FileWatcher: public QObject
{
    Q_OBJECT


public:
    FileWatcher(QObject *parent = nullptr, QString file = "");
    const QString &filePath() const;
    void setFilePath(const QString &newFilePath);

    const QString &parentPath() const;

    bool running() const;

public slots:
    void start();
    void stop();
private slots:
//    void onFileAdded(const std::wstring& file);
//    void onFileRemoved(const std::wstring& file);
//    void onFileChanged(const std::wstring& file);
//    void onFileRenamed(const std::wstring& file);

signals:
    void fileChanged(bool);
private:
    QString filePath_;
    QAtomicInteger<int> running_;
};
}

#endif // FILEWATCHER_H
