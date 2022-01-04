#ifndef LOGEVENT_H
#define LOGEVENT_H


#include <QString>

namespace Yate {

enum class LogEventType {
    NightBegin,
    TeralystSpawn,
    DayBegin,
    LimbBreak,
    EidolonCapture,
    EidolonKill,
    LootDrop,
    ShrineEnable,
    ShardInsert,
    ShardRemove,
    ShrineDisable,
    EidolonSpawn,
    HostUnload,
    Invalid,
    HostJoin,
    SquadJoin,
    EidolonDespawn
};



class LogEvent
{
public:
    LogEvent(int eId = -1, LogEventType eType = LogEventType::Invalid, float eTimestamp = -1, int eValue = -1, QString eStrVal = "");
    int id() const;
    void setId(int newId);

    LogEventType type() const;
    void setType(LogEventType newType);

    float timestamp() const;
    void setTimestamp(float newTimestamp);

    int value() const;
    void setValue(int newValue);

    const QString &strValue() const;
    void setStrValue(const QString &newStrValue);

private:
    int id_;
    LogEventType type_;
    float timestamp_;
    int value_;
    QString strValue_;
};

}

#endif // LOGEVENT_H
