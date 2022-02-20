#include "logevent.h"
#include <QMap>
namespace Yate {


LogEvent::LogEvent(int eId, LogEventType eType, float eTimestamp, int eValue, QString eStrVal):id_(eId), type_(eType), timestamp_(eTimestamp), value_(eValue), strValue_(eStrVal)
{



}

int LogEvent::id() const
{
    return id_;
}

void LogEvent::setId(int newId)
{
    id_ = newId;
}

LogEventType LogEvent::type() const
{
    return type_;
}

void LogEvent::setType(LogEventType newType)
{
    type_ = newType;
}

float LogEvent::timestamp() const
{
    return timestamp_;
}

void LogEvent::setTimestamp(float newTimestamp)
{
    timestamp_ = newTimestamp;
}

int LogEvent::value() const
{
    return value_;
}

void LogEvent::setValue(int newValue)
{
    value_ = newValue;
}

const QString &LogEvent::strValue() const
{
    return strValue_;
}

void LogEvent::setStrValue(const QString &newStrValue)
{
    strValue_ = newStrValue;
}

QString LogEvent::typeName() const
{
    static QMap<LogEventType, QString> logEvtTypName{
            {LogEventType::NightBegin, "NightBegin"},
            {LogEventType::DayBegin, "DayBegin"},
            {LogEventType::TeralystSpawn, "TeralystSpawn"},
            {LogEventType::LimbBreak, "LimbBreak"},
            {LogEventType::EidolonCapture, "EidolonCapture"},
            {LogEventType::EidolonKill, "EidolonKill"},
            {LogEventType::LootDrop, "LootDrop"},
            {LogEventType::ShrineEnable, "ShrineEnable"},
            {LogEventType::ShardInsert, "ShardInsert"},
            {LogEventType::ShardRemove, "ShardRemove"},
            {LogEventType::ShrineDisable, "ShrineDisable"},
            {LogEventType::EidolonSpawn, "EidolonSpawn"},
            {LogEventType::HostJoin, "HostJoin"},
            {LogEventType::SquadJoin, "SquadJoin"},
            {LogEventType::HostUnload, "HostUnload"},
            {LogEventType::EidolonDespawn, "EidolonDespawn"},
            {LogEventType::Invalid, "Invalid"}
    };
    return logEvtTypName.value(type_, "Invalid");
}

}
