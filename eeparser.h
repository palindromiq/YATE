#ifndef EEPARSER_H
#define EEPARSER_H
#include <QObject>
#include <QTextStream>
#include "logevent.h"

namespace Yate {
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


    static LogEventType msgToEventType(QString msg, int &val);
public slots:
    void start();
    void reset();

signals:
    void parsingStarted();
    void parsingFinished();
    void parsingError(QString);
    void logEvent(LogEvent &e);
private:
    QString filename_;
    bool liveParsing_;
    int currentPosition_;
    int evtId_;
};
}

#endif // EEPARSER_H
