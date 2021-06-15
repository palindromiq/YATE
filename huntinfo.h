#ifndef HUNTINFO_H
#define HUNTINFO_H

#include <QVector>

namespace Yate {

enum class CapState {
    Capture,
    Kill,
    Incomplete
};

enum class Eidolon {
    Terralyst,
    Gantulyst,
    Hydrolyst
};

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

    float healingPhaseTime() const;
    void setHealingPhaseTime(float newHealingPhaseTime);

    bool spawned() const;
    void setSpawned(bool newSpawned);

    CapState result() const;
    void setResult(CapState newResult);

    Eidolon eidolon() const;
    void setEidolon(Eidolon newEidolon);

private:
    bool valid_;
    float shrineTime_;
    QVector<float> timeBetweenShards_;
    float spawnDelay_;
    float waterShield_;
    float shrineActivationTime_;
    float capshotTime_;
    float healingPhaseTime_;
    int numberOfLimbs_;
    bool spawned_;
    CapState result_;
    Eidolon eidolon_;
};

class RunInfo {
public:
    RunInfo();
    const CapInfo &terralystCapInfo() const;
    void setTerralystCapInfo(const CapInfo &newTerralystCapInfo);

    const CapInfo &gantulystCapInfo() const;
    void setGantulystCapInfo(const CapInfo &newGantulystCapInfo);

    const CapInfo &hydrolystCapInfo() const;
    void setHydrolystCapInfo(const CapInfo &newHydrolystCapInfo);

    void clear();

private:
    CapInfo terralystCapInfo_;
    CapInfo gantulystCapInfo_;
    CapInfo hydrolystCapInfo_;
};

class NightInfo {
public:
    NightInfo();
    const QVector<RunInfo> &runs() const;
    void setRuns(const QVector<RunInfo> &newRuns);
    void addRun(RunInfo runInfo);
    RunInfo run(int index);
    void removeRun(int index);
    void clear();

private:
    QVector<RunInfo> runs_;
};

class HuntInfo
{
public:
    HuntInfo();
    const QVector<NightInfo> &nights() const;
    void setNights(const QVector<NightInfo> &newNights);
    void addNight(NightInfo nightInfo);
    NightInfo night(int index);
    void removeNight(int index);
    void clear();
private:
    QVector<NightInfo> nights_;
};
}

#endif // HUNTINFO_H
