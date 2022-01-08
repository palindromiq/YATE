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


    if (argc >= 5) {
        QString action = argv[1];


        // ({"update", QString::number(QCoreApplication::applicationPid()),latestVersion_});
        if (action == "update") {
            //pid = argv[2]
            QString version = argv[3];
            for(int i = 4; i < argc; i++) {
                QString oldPath = argv[i];
                if(!QFile::remove(oldPath)) {
                    QTimer::singleShot(5000, [=](){
                        QFile::remove(oldPath);
                    });
                }
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
    bool clientVersion = false;
    if (argc > 1 && QString(argv[1]) == "client") {
        clientVersion = true;
    }
#ifdef YATE_CLIENT_VERSION
    clientVersion = true;
#endif
    Yate::YATEWindow w(clientVersion);
    w.show();
    return a.exec();
}
