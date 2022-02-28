#ifndef HUNTINFOGENERATOR_H
#define HUNTINFOGENERATOR_H

#include <QObject>
#include <QSet>
#include "logevent.h"

class QSettings;

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
    void emitLimbsUpdate();


public slots:
    void onLogEvent(LogEvent &e);
    void resetHuntInfo();

signals:
    void onHuntStateChanged(QString);
    void onLimbsChanged(QString);
    void onHostChanged(QString);
    void onSquadChanged(QSet<QString>);
    void onHostOrSquadChanged(QString);

private:
    HuntInfo* huntInfo_;
    QSettings *settings_;
    HuntState state_;
    float lastEventTime_;
    float firstSetStartTime_;
    LogEvent lastEvent_;
    int currentCapIndex_;
    int currentRunIndex_;
    int currentNightIndex_;
    bool nightEnded_;
    float shrineDelay_;
    float doorOpeningTimestamp_;
    bool showLimbsSummary_;
    bool showLimbsSummaryAfterLast_;
    int limbsSummaryPrec_;
    QString host_;
    QSet<QString> squadBuffer_;
};
}

#endif // HUNTINFOGENERATOR_H
