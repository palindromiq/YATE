#ifndef HUNTIMAGEGENERATOR_H
#define HUNTIMAGEGENERATOR_H

#include <QObject>
#include "huntinfo.h"

namespace Yate {
class HuntImageGenerator : public QObject
{
    Q_OBJECT
public:
    explicit HuntImageGenerator(NightInfo &night, QString host, QSet<QString> squad, QObject *parent = nullptr);
    bool saveImage(QString path) const;

signals:
private:
    NightInfo night_;
    QString host_;
    QSet<QString> squad_;

};
}

#endif // HUNTIMAGEGENERATOR_H
