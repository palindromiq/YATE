#include "yatewindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>


#include "globals.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(SETTINGS_ORGANIZATION);
    QCoreApplication::setApplicationName(SETTINGS_APPLICATION);
    QFile file(":/dark.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    a.setStyleSheet(stream.readAll());
    Yate::YATEWindow w;
    w.show();
    return a.exec();
}
