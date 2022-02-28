#ifndef TIMELINEIMAGEGENERATOR_H
#define TIMELINEIMAGEGENERATOR_H

#include <QObject>
#include <QImage>
#include "huntinfo.h"

namespace Yate {
class TimelineImageGenerator : public QObject
{
    Q_OBJECT
public:
    explicit TimelineImageGenerator(QString path, NightInfo &night, QString host, QSet<QString> squad, QObject *parent = nullptr);


signals:
    void exportFinished(bool);
    void generateFinished(QImage);


public slots:
    QImage *generateImage();
    void exportImage();
    void generateAndEmit();
private:
    QString path_;
    NightInfo night_;
    QString host_;
    QSet<QString> squad_;
};
}

#endif // TIMELINEIMAGEGENERATOR_H
