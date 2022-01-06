#include "yatewindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTimer>

#include "globals.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc >= 2) {
        QString action = argv[1];
        if (action == "update") {
            QString oldPath = argv[2];
            QString version = argv[4];
            if(!QFile::remove(oldPath)) {
                QTimer::singleShot(5000, [=](){QFile::remove(oldPath);});
            }
            QMessageBox::information(0, "Updated Successfully", "Successfully updated YATE to version " + version + ". To learn about the latest features visit " + "<a href='" + SETTINGS_WEBSITE_HTTPS  + "'>" + SETTINGS_WEBSITE_HTTPS  + "</a>");
        }
    }



    QCoreApplication::setOrganizationName(SETTINGS_ORGANIZATION);
    QCoreApplication::setApplicationName(SETTINGS_APPLICATION);
    QFile file(":/MaterialDark.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    a.setStyleSheet(stream.readAll());
    Yate::YATEWindow w;
    w.show();
    return a.exec();
}
