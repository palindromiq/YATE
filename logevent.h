#ifndef LOGEVENT_H
#define LOGEVENT_H




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
    EidolonDespawn
};



class LogEvent
{
public:
    LogEvent(int eId = -1, LogEventType eType = LogEventType::Invalid, float eTimestamp = -1, int eValue = -1);
    int id() const;
    void setId(int newId);

    LogEventType type() const;
    void setType(LogEventType newType);

    float timestamp() const;
    void setTimestamp(float newTimestamp);

    int value() const;
    void setValue(int newValue);

private:
    int id_;
    LogEventType type_;
    float timestamp_;
    int value_;
};

}

#endif // LOGEVENT_H
