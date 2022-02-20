#ifndef HUNTINFO_H
#define HUNTINFO_H

#include <QVector>
#include <QSet>
#include "globals.h"
#include "logevent.h"

namespace Yate {
class AnalysisViewItem;



enum class CapState {
    Capture,
    Kill,
    Spawned,
    InComplete
};

enum class Eidolon {
    Teralyst,
    Gantulyst,
    Hydrolyst
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
class CapInfo {
public:
    CapInfo();
    bool valid() const;
    void setValid(bool newValid);

    float shrineTime() const;
    void setShrineTime(float newShrineTime);

    const QVector<float> &timeBetweenShards() const;
    void setTimeBetweenShards(const QVector<float> &newTimeBetweenShards);
    void addTimeBetweenShards(float &newTimeBetweenShards);
    float timeBetweenShards(int index) const;
    void clearTimeBetweenShards();

    const QVector<float> &limbBreaks() const;
    void setlimbBreaks(const QVector<float> &newLimbBreaks);
    void addlimbBreak(float &newLimbBreak);
    float limbBreak(int index) const;
    void clearLimbBreaks();

    float spawnDelay() const;
    void setSpawnDelay(float newSpawnDelay);

    float waterShield() const;
    void setWaterShield(float newWaterShield);

    float shrineActivationTime() const;
    void setShrineActivationTime(float newShrineActivationTime);

    float capshotTime() const;
    void setCapshotTime(float newCapshotTime);

    int numberOfLimbs() const;
    void setNumberOfLimbs(int newNumberOfLimbs);

    CapState result() const;
    void setResult(CapState newResult);

    Eidolon eidolon() const;
    void setEidolon(Eidolon newEidolon);

    AnalysisViewItem *toAnalysisViewItem() const;

    float lastLimbProgressTime() const;
    void setLastLimbProgressTime(float newLastLimbTime);

    float loadTime() const;
    void setLoadTime(float newLoadTime);

    float lootDropTime() const;
    void setLootDropTime(float newLootDropTime);

    float lootDropTimestamp() const;
    void setLootDropTimestamp(float newLootDropTimestamp);

    float spawnTimestamp() const;
    void setSpawnTimestamp(float newSpawnTimestamp);

    float capshotProgressTimestamp() const;
    void setCapshotProgressTime(float newCapshotTimestamp);

    bool lateShardInsertLog() const;
    void setLateShardInsertLog(bool newLateShardInsertLog);

private:
    bool valid_;
    float shrineTime_;
    QVector<float> timeBetweenShards_;
    QVector<float> limbBreaks_;
    float spawnDelay_;
    float waterShield_;
    float shrineActivationTime_;
    float capshotTime_;
    float lastLimbProgessTime_;
    float loadTime_;
    float lootDropTime_;
    float lootDropTimestamp_;
    float spawnTimestamp_;
    float capshotProgressTimestamp_;
    int numberOfLimbs_;
    bool lateShardInsertLog_;
    CapState result_;
    Eidolon eidolon_;
};

class RunInfo {
public:
    RunInfo();
    CapInfo &teralystCapInfo();
    void setTeralystCapInfo(const CapInfo &newTeralystCapInfo);

    CapInfo &gantulystCapInfo();
    void setGantulystCapInfo(const CapInfo &newGantulystCapInfo);

    CapInfo &hydrolystCapInfo();
    void setHydrolystCapInfo(const CapInfo &newHydrolystCapInfo);

    CapInfo &capInfoByIndex(int index);



    void clear();

    AnalysisViewItem *toAnalysisViewItem(int runNo);
    QString getRunResult();

    int getNumberOfCaps();
    int getNumberOfKills();


    float startTimestamp() const;
    void setStartTimestamp(float newStartTime);

    const QVector<LogEvent> &eventLog() const;
    void addEvent(const LogEvent &evt);

private:
    CapInfo teralystCapInfo_;
    CapInfo gantulystCapInfo_;
    CapInfo hydrolystCapInfo_;
    float startTimestamp_;
    QVector<LogEvent> eventLog_;
};

class NightInfo {
public:
    NightInfo();
    const QVector<RunInfo> &runs() const;
    void setRuns(const QVector<RunInfo> &newRuns);
    void addRun(RunInfo runInfo);
    RunInfo &run(int index);
    void removeRun(int index);
    void clear();

    AnalysisViewItem *toAnalysisViewItem(int nightNo) const;
    QString getNightResult() const;
    QString getAverage() const;
    int getNumberOfHydrolysts() const;
    int validRunCount() const;


    float startTimestamp() const;
    void setStartTimestamp(float newStartTime);

    void addSquadMember(const QString &member);
    void addSquadMembers(const QSet<QString> &members);
    void removeSquadMember(const QString &member);

    QString squadString() const;


    const QString &host() const;
    void setHost(const QString &newHost);

    const QSet<QString> &squad() const;
    void setSquad(const QSet<QString> &newSquad);

private:
    QVector<RunInfo> runs_;
    float startTimestamp_;
    QString host_;
    QSet<QString> squad_;
};

class HuntInfo
{
public:
    HuntInfo();
    const QVector<NightInfo> &nights() const;
    void setNights(const QVector<NightInfo> &newNights);
    int nightCount() const;
    void addNight(NightInfo nightInfo);
    NightInfo &night(int index);
    void removeNight(int index);
    void clear();

    static QString eidolonName(Eidolon eido, bool abbreviate = false);
    static QString eidolonName(int eido, bool abbreviate = false);
    static QString timestampToProgressString(float timestamp);

    QVector<AnalysisViewItem *> toAnalysisViewItem() const;

private:
    QVector<NightInfo> nights_;
};

#pragma GCC diagnostic pop
}

#endif // HUNTINFO_H
