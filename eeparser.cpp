#include "eeparser.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDir>

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
    if(watcher_) {
        watcher_->removePath(filename());
    }
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
        for(auto &line: lines) {
             parseLine(line);
        }
    } else {
        emit parsingError(logFile.errorString());
    }

    emit parsingFinished();

}

void EEParser::startLive()
{
    emit parsingStarted();
    QFile logFile(filename());
    watcher_ = new QFileSystemWatcher(this);
    parentWatcher_ = new QFileSystemWatcher(this);
    watcher_->addPath(filename());
    QString fileContent;
    QString parentPath = QFileInfo(logFile).dir().path();
    parentWatcher_->addPath(parentPath);
    connect(parentWatcher_, &QFileSystemWatcher::directoryChanged,  this, &EEParser::onDirectoryChanged);
    if(!logFile.exists()) {
        logDoesNotExist_ = true;

        connect(watcher_, &QFileSystemWatcher::fileChanged, this, &EEParser::onFileChanged);
    } else if(logFile.open(QIODevice::ReadOnly)) {
        logDoesNotExist_ = false;
        fileContent =  QString(logFile.readAll());
        auto lines = fileContent.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
        for(auto &line: lines) {
             parseLine(line);
        }
        setCurrentPosition(fileContent.length());
        connect(watcher_, &QFileSystemWatcher::fileChanged, this, &EEParser::onFileChanged);
    } else {
        emit parsingError(logFile.errorString());
    }

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

void EEParser::onFileChanged(QString path)
{
    if(!watcher_->files().contains(path)) {
        if (QFile(path).exists()) {
            watcher_->addPath(path);
            setCurrentPosition(0);
        } else {
            emit parsingError(tr("Log file not found!"));
        }
    }
    QFile logFile(path);
    QString fileContent;

    if(logFile.open(QIODevice::ReadOnly)) {
        logFile.seek(currentPosition());
        fileContent =  QString(logFile.readAll());
        setCurrentPosition(currentPosition() + fileContent.length());
        auto lines = fileContent.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
        for(auto &line: lines) {
             parseLine(line);
        }
    } else {
        emit parsingError(logFile.errorString());
    }


}

void EEParser::onDirectoryChanged(QString)
{
    QFile logFile(filename());
    if (logFile.exists()) {
        setCurrentPosition(0);
        emit parsingReset();
        if (!watcher_->files().contains(filename())) {
            watcher_->addPath(filename());
        }
        onFileChanged(filename());
        logDoesNotExist_ = false;
    } else {
        setCurrentPosition(0);
        logDoesNotExist_ = true;
        emit parsingReset();
    }
}

}
