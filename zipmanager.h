#ifndef ZIPMANAGER_H
#define ZIPMANAGER_H

#include <QObject>

namespace Yate {

class ZipManager : public QObject
{
    Q_OBJECT
public:
    explicit ZipManager(QObject *parent = nullptr);
    bool unzip(QString sourceArchive, QString destDir);

signals:

};
}

#endif // ZIPMANAGER_H
