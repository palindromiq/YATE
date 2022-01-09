#include "yatewindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTimer>


#include "globals.h"

#ifdef QT_DEBUG
QFile loggingFile;
void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QTextStream& qStdErr()
{
    static QTextStream ts(stderr);
    return ts;
}
QTextStream& qStdOut()
{
    static QTextStream ts(stdout);
    return ts;
}
#endif



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef QT_DEBUG
    qInstallMessageHandler(logMessageHandler);
    qDebug() << "YATE Launched";
    QStringList args;
    for(int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    qDebug() << "Version: " << QApplication::applicationVersion();
    qDebug() << "Arguments: " << args.join(", ");
#endif

    if (argc >= 5) {
        QString action = argv[1];


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

    Yate::YATEWindow w;
    w.show();
    return a.exec();
}


#ifdef QT_DEBUG
void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString logType = "System";
    bool isStdOut = false;
    switch (type) {
    case QtDebugMsg:
        logType = "Debug";
        isStdOut = true;
        break;
    case QtInfoMsg:
        logType = "Info";
        isStdOut = true;
        break;
    case QtWarningMsg:
        logType = "Warning";
        break;
    case QtCriticalMsg:
        logType = "Critical";
        break;
    case QtFatalMsg:
        logType = "Fatal";
        break;
    }
    QString txt = QString("[%1] %2: %3 (%4:%5, %6)").arg(QDateTime::currentDateTime().toString()).arg(logType).arg(localMsg.constData()).arg(file).arg(context.line).arg(function);
    if (isStdOut) {
        qStdOut() << txt.toLocal8Bit().data() << Qt::endl;
    } else {
        qStdErr() << txt.toLocal8Bit().data() << Qt::endl;
    }
    QFile outFile("yate.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
}
#endif

