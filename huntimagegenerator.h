#ifndef HUNTIMAGEGENERATOR_H
#define HUNTIMAGEGENERATOR_H

#include <QObject>
#include "huntinfo.h"

namespace Yate {
class HuntImageGenerator : public QObject
{
    Q_OBJECT
public:
    explicit HuntImageGenerator(QString path, NightInfo &night, QString host, QSet<QString> squad, QObject *parent = nullptr);

signals:
    void generationFinished(bool);

public slots:
    void generateImage();
private:
    QString path_;
    NightInfo night_;
    QString host_;
    QSet<QString> squad_;
};
}

#endif // HUNTIMAGEGENERATOR_H
