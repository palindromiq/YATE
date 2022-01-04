#include "huntimagegenerator.h"

#include <QDebug>

namespace Yate {
HuntImageGenerator::HuntImageGenerator(NightInfo &night, QString host, QSet<QString> squad, QObject *parent)
    : QObject{parent}, night_(night), host_(host), squad_(squad)
{
    qDebug() << night_.getAverage();
}

bool HuntImageGenerator::saveImage(QString path) const
{
    return true;
}

}
