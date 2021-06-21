#include "eeparser.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

namespace Yate {

EEParser::EEParser(QString logFilename, bool isLive, QObject *parent):QObject(parent), filename_(logFilename), liveParsing_(isLive), currentPosition_(0), evtId_(0)
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

void EEParser::start()
{
    emit parsingStarted();
    QFile logFile(filename());
    QString line;
    QRegularExpression rx("(\\d+\\.\\d+)\\s+(\\w+)\\s+\\[(\\w+)\\]\\s*\\:\\s*(.*)");



    if(logFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&logFile);
        while(!stream.atEnd()) {
             stream.readLineInto(&line);
             setCurrentPosition(currentPosition() + line.size());
             line = line.trimmed();


             auto it = rx.globalMatch(line);
             bool matched = false;
             float timestamp;
             QString msgText;
             if (it.hasNext()) {
                 matched = true;
                 auto match = it.next();
                 timestamp = match.captured(1).toFloat();
                 msgText = match.captured(4).trimmed();
             }
//             auto parts = line.split(":");
//             auto precolon = parts[0];
//             parts.removeFirst();
//             auto msgText = parts.join(":").trimmed();
//             float timestamp = (precolon.split(" "))[0].trimmed().toFloat();
//             bool matched = true;
             if(matched) {
                 int val;
                 auto evtType = EEParser::msgToEventType(msgText, val);
                 if (evtType != LogEventType::Invalid) {
                     LogEvent evt(evtId_++, evtType, timestamp, val);
                     emit logEvent(evt);
                 }
             }

        }
    } else {
        qCritical() << logFile.errorString();
    }

    if (!liveParsing()) {
        emit parsingFinished();
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
        {"EidolonMP.lua: EIDOLONMP: Level fully destroyed", LogEventType::HostUnload}
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
        const QRegularExpression rx("[^0-9]+");
        const auto&& parts = msg.split(rx, Qt::SkipEmptyParts);
        val = parts[0].toInt();
        return LogEventType::ShardRemove;
    } else {
        return LogEventType::Invalid;
    }

}

}
