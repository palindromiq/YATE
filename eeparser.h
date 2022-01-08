#ifndef EEPARSER_H
#define EEPARSER_H
#include <QObject>
#include <QTextStream>
#include <QRegularExpression>
#include "logevent.h"

class QFileSystemWatcher;

namespace Yate {
class FileWatcher;
class EEParser: public QObject
{
    Q_OBJECT
public:
    EEParser(QString logFilename, bool isLive = false, QObject *parent = nullptr);
    bool liveParsing() const;
    void setLiveParsing(bool newLiveParsing);
    const QString &filename() const;
    void setFilename(const QString &newFilename);

    int currentPosition() const;
    void setCurrentPosition(int newCurrentPosition);


    static LogEventType msgToEventType(QString msg, int &val, QString& strVal);

private slots:
    void onFileChanged(bool exist);

public slots:
    void startOffline();
    void startLive();
    void reset();
    void parseLine(QString &line);
    void stopParsing();

signals:
    void parsingStarted();
    void parsingFinished();
    void parsingReset();
    void parsingError(QString);
    void logEvent(Yate::LogEvent &e);

    void startWatcher();
    void stopWatcher();
private:
    QString filename_;
    bool liveParsing_;
    int currentPosition_;
    int evtId_;
    QRegularExpression lineParseRegex_;
    FileWatcher *watcher_;
    bool logDoesNotExist_;
    bool hostJustUnloaded_;
};
}

#endif // EEPARSER_H
