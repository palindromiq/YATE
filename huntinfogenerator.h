#ifndef HUNTINFOGENERATOR_H
#define HUNTINFOGENERATOR_H

#include <QObject>
#include "logevent.h"


namespace Yate  {
class HuntInfo;
class NightInfo;
class RunInfo;
class CapInfo;

enum class HuntStateStage {
    Initial,
    TerralytSpawn,
    Spawned,
    Limbs,
    Healing,
    Capped,
    Killed,
    LootDropped,
    ShrineEnabled,
    ShardInserted,
};

enum class HuntStateChangeType {
    Watershield,
    LimbBreak,
    CapShot
};

class HuntState {

public:
    HuntState();
    HuntStateStage stage() const;
    void setStage(HuntStateStage newStage);

    int limbNumber() const;
    void setLimbNumber(int newLimbNumber);

    int eidolonNumber() const;
    void setEidolonNumber(int newEidolonNumber);

    void reset();

private:
    HuntStateStage stage_;
    int limbNumber_;
    int eidolonNumber_;
};

class HuntInfoGenerator: public QObject
{
    Q_OBJECT
public:
    HuntInfoGenerator(QObject *parent = nullptr);
    HuntInfo *huntInfo() const;


    ~HuntInfoGenerator();

    float getLastEventTime() const;
    void setLastEventTime(float newLastEventTime);

public slots:
    void onLogEvent(LogEvent &e);
    void resetHuntInfo();

signals:
    void onHuntStateChanged(QString);

private:
    HuntInfo* huntInfo_;
    HuntState state_;
    float lastEventTime_;
    float firstSetStartTime_;
    LogEvent lastEvent_;
    int currentCapIndex_;
    int currentRunIndex_;
    int currentNightIndex_;
    bool nightEnded_;
    float shrineDelay_;
};
}

#endif // HUNTINFOGENERATOR_H
