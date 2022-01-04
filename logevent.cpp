#include "logevent.h"

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

}
