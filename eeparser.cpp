#include "eeparser.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QThread>


#include "filewatcher.h"

namespace Yate {

EEParser::EEParser(QString logFilename, bool isLive, QObject *parent):
    QObject(parent), filename_(logFilename), liveParsing_(isLive), currentPosition_(0), evtId_(0), lineParseRegex_("(\\d+\\.\\d+)\\s+(\\w+)\\s+\\[(\\w+)\\]\\s*\\:\\s*(.*)"), logDoesNotExist_(true)
{

}

bool EEParser::liveParsing() const
{
    return liveParsing_;
}

void EEParser::setLiveParsing(bool newLiveParsing)
{
    liveParsing_ = newLiveParsing;
}

const QString &EEParser::filename() const
{
    return filename_;
}

void EEParser::setFilename(const QString &newFilename)
{
    filename_ = newFilename;
}

int EEParser::currentPosition() const
{
    return currentPosition_;
}

void EEParser::setCurrentPosition(int newCurrentPosition)
{
    currentPosition_ = newCurrentPosition;
}

void EEParser::reset()
{
    setCurrentPosition(0);
    evtId_ = 0;
}

void Yate::EEParser::parseLine(QString &line)
{
    line = line.trimmed();
    if (!line.length()) {
        return;
    }
    auto it = lineParseRegex_.globalMatch(line);
    bool matched = false;
    float timestamp;
    QString msgText;
    if (it.hasNext()) {
        matched = true;
        auto match = it.next();
        timestamp = match.captured(1).toFloat();
        msgText = match.captured(4).trimmed();
    }

    if(matched) {
        int val;
        auto evtType = EEParser::msgToEventType(msgText, val);
        if (evtType != LogEventType::Invalid) {
            LogEvent evt(evtId_++, evtType, timestamp, val);
            emit logEvent(evt);
        }
    }
}

void EEParser::stopParsing()
{
    emit stopWatcher();
    emit parsingFinished();
}

void EEParser::startOffline()
{
    emit parsingStarted();
    QFile logFile(filename());
    QString fileContent;

    if(logFile.open(QIODevice::ReadOnly)) {
        fileContent =  QString(logFile.readAll());
        auto lines = fileContent.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
        for(int i = 0; i < lines.size(); i++) {
            parseLine(lines[i]);
        }
        logFile.close();
    } else {
        emit parsingError(logFile.errorString());
    }

    emit parsingFinished();

}


void EEParser::startLive()
{
    emit parsingStarted();
    QFile logFile(filename());
    watcher_ = new FileWatcher(nullptr, filename());
    QThread *watcherThread = new QThread;

    watcher_->moveToThread(watcherThread);

    connect( watcherThread, &QThread::started, watcher_, &FileWatcher::start);
    connect( this, &EEParser::parsingFinished, watcher_, &FileWatcher::stop, Qt::DirectConnection);
    connect( watcherThread, &QThread::finished, watcher_, &QThread::deleteLater);
    connect( watcherThread, &QThread::finished, watcherThread, &QThread::deleteLater);


    connect(watcher_, &FileWatcher::fileChanged, this, &EEParser::onFileChanged);

    QTimer *tmr = new QTimer(this);
    tmr->setInterval(1000);
    connect(tmr, &QTimer::timeout, [&]() {if(QFileInfo::exists(filename())) { QFileInfo(filename()).lastModified();}});
    tmr->start();

    QString fileContent;


    if(logFile.open(QIODevice::ReadOnly)) {
        fileContent =  QString(logFile.readAll());
        auto lines = fileContent.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
        for(int i = 0; i < lines.size(); i++) {
            parseLine(lines[i]);
        }
        setCurrentPosition(fileContent.length());
        logFile.close();
    } else {
        emit parsingError(logFile.errorString());
    }
    watcherThread->start();

}

LogEventType EEParser::msgToEventType(QString msg, int &val)
{
    QMap<QString, LogEventType> msgEvtMap{
        {"TeralystEncounter.lua: It's nighttime!", LogEventType::NightBegin},
        {"TeralystEncounter.lua: It's daytime!", LogEventType::DayBegin},
        {"TeralystAvatarScript.lua: Weakpoint Destroyed", LogEventType::LimbBreak},
        {"TeralystAvatarScript.lua: Teralyst Captured", LogEventType::EidolonCapture},
        {"TeralystAvatarScript.lua: Teralyst Killed", LogEventType::EidolonKill},
        {"SnapPickupToGround.lua: Snapping pickup to ground (DefaultArcanePickup)", LogEventType::LootDrop},
        {"TeralystEncounter.lua: Shrine enabled", LogEventType::ShrineEnable},
//        {"TeralystEncounter.lua: Shrine disabled", LogEventType::ShrineDisable},
        {"EidolonMP.lua: EIDOLONMP: Finalize Eidolon transition", LogEventType::TerralystSpawn},
        {"TeralystEncounter.lua:      Eidolon spawning SUCCESS", LogEventType::EidolonSpawn},
        {"EidolonMP.lua: EIDOLONMP: Level fully destroyed", LogEventType::HostUnload},
        {"TeralystEncounter.lua: Teralyst Escape complete. All Teralysts should be gone now", LogEventType::EidolonDespawn}
    };
    val = -1;

    if (msgEvtMap.contains(msg)) {
        return msgEvtMap[msg];
    } else if (msg.startsWith("TeralystEncounter.lua: A shard has been put in the Eidolon Shrine.")) {
        const QRegularExpression rx("[^0-9]+");
        const auto&& parts = msg.split(rx, Qt::SkipEmptyParts);

        val = parts[0].toInt();
        return LogEventType::ShardInsert;
    } else if (msg.startsWith("TeralystEncounter.lua: A shard has been removed from the Eidolon Shrine.")) {
//        const QRegularExpression rx("[^0-9]+");
//        const auto&& parts = msg.split(rx, Qt::SkipEmptyParts);
//        val = parts[0].toInt();
//        return LogEventType::ShardRemove;
        return LogEventType::Invalid;
    } else {
        return LogEventType::Invalid;
    }

}

void EEParser::onFileChanged(bool exists)
{
    if (!exists) {
        setCurrentPosition(0);
        emit parsingReset();
    } else {
        QFile logFile(filename());
        QString fileContent;
        if(logFile.open(QIODevice::ReadOnly)) {
            logFile.seek(currentPosition());
            fileContent =  QString(logFile.readAll());
            auto lines = fileContent.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
            for(int i = 0; i < lines.size() - 1; i++) {
                parseLine(lines[i]);
            }
            if (lines.size()) {
                setCurrentPosition(currentPosition() + fileContent.length() - lines[lines.size() - 1].size());
            }
            logFile.close();
        } else {
            emit parsingError(logFile.errorString());
        }
    }

}

}
