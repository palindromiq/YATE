#include "huntinfo.h"




namespace Yate {

HuntInfo::HuntInfo()
{

}
const QVector<NightInfo> &HuntInfo::nights() const
{
    return nights_;
}

void HuntInfo::setNights(const QVector<NightInfo> &newNights)
{
    nights_ = newNights;
}

void HuntInfo::addNight(NightInfo nightInfo)
{
    nights_.push_back(nightInfo);
}

NightInfo HuntInfo::night(int index)
{
    return nights_.at(index);
}

void HuntInfo::removeNight(int index)
{
    nights_.removeAt(index);
}

void HuntInfo::clear()
{
    nights_.clear();
}

NightInfo::NightInfo()
{

}

const QVector<RunInfo> &NightInfo::runs() const
{
    return runs_;
}

void NightInfo::setRuns(const QVector<RunInfo> &newRuns)
{
    runs_ = newRuns;
}

void NightInfo::addRun(RunInfo runInfo)
{
    runs_.push_back(runInfo);
}

RunInfo NightInfo::run(int index)
{
    return runs_.at(index);
}

void NightInfo::removeRun(int index)
{
    runs_.removeAt(index);
}

void NightInfo::clear()
{
    runs_.clear();
}

RunInfo::RunInfo()
{

}

const CapInfo &RunInfo::terralystCapInfo() const
{
    return terralystCapInfo_;
}

void RunInfo::setTerralystCapInfo(const CapInfo &newTerralystCapInfo)
{
    terralystCapInfo_ = newTerralystCapInfo;
}

const CapInfo &RunInfo::gantulystCapInfo() const
{
    return gantulystCapInfo_;
}

void RunInfo::setGantulystCapInfo(const CapInfo &newGantulystCapInfo)
{
    gantulystCapInfo_ = newGantulystCapInfo;
}

const CapInfo &RunInfo::hydrolystCapInfo() const
{
    return hydrolystCapInfo_;
}

void RunInfo::setHydrolystCapInfo(const CapInfo &newHydrolystCapInfo)
{
    hydrolystCapInfo_ = newHydrolystCapInfo;
}

void RunInfo::clear()
{
    CapInfo emptyCap;
    setTerralystCapInfo(emptyCap);
    setGantulystCapInfo(emptyCap);
    setHydrolystCapInfo(emptyCap);
}

CapInfo::CapInfo():valid_(false)
{

}

bool CapInfo::valid() const
{
    return valid_;
}

void CapInfo::setValid(bool newValid)
{
    valid_ = newValid;
}

float CapInfo::shrineTime() const
{
    return shrineTime_;
}

void CapInfo::setShrineTime(float newShrineTime)
{
    shrineTime_ = newShrineTime;
}

const QVector<float> &CapInfo::timeBetweenShards() const
{
    return timeBetweenShards_;
}

void CapInfo::setTimeBetweenShards(const QVector<float> &newTimeBetweenShards)
{
    timeBetweenShards_ = newTimeBetweenShards;
}

void CapInfo::addTimeBetweenShards(float &newTimeBetweenShards)
{
    timeBetweenShards_.append(newTimeBetweenShards);
}

float CapInfo::timeBetweenShards(int index) const
{
    return timeBetweenShards_.at(index);
}

void CapInfo::clearTimeBetweenShards()
{
    timeBetweenShards_.clear();
}

float CapInfo::spawnDelay() const
{
    return spawnDelay_;
}

void CapInfo::setSpawnDelay(float newSpawnDelay)
{
    spawnDelay_ = newSpawnDelay;
}

float CapInfo::waterShield() const
{
    return waterShield_;
}

void CapInfo::setWaterShield(float newWaterShield)
{
    waterShield_ = newWaterShield;
}

float CapInfo::shrineActivationTime() const
{
    return shrineActivationTime_;
}

void CapInfo::setShrineActivationTime(float newShrineActivationTime)
{
    shrineActivationTime_ = newShrineActivationTime;
}

float CapInfo::capshotTime() const
{
    return capshotTime_;
}

void CapInfo::setCapshotTime(float newCapshotTime)
{
    capshotTime_ = newCapshotTime;
}

int CapInfo::numberOfLimbs() const
{
    return numberOfLimbs_;
}

void CapInfo::setNumberOfLimbs(int newNumberOfLimbs)
{
    numberOfLimbs_ = newNumberOfLimbs;
}

float CapInfo::healingPhaseTime() const
{
    return healingPhaseTime_;
}

void CapInfo::setHealingPhaseTime(float newHealingPhaseTime)
{
    healingPhaseTime_ = newHealingPhaseTime;
}

bool CapInfo::spawned() const
{
    return spawned_;
}

void CapInfo::setSpawned(bool newSpawned)
{
    spawned_ = newSpawned;
}

CapState CapInfo::result() const
{
    return result_;
}

void CapInfo::setResult(CapState newResult)
{
    result_ = newResult;
}

Eidolon CapInfo::eidolon() const
{
    return eidolon_;
}

void CapInfo::setEidolon(Eidolon newEidolon)
{
    eidolon_ = newEidolon;
    if (eidolon_ == Eidolon::Terralyst) {
        setNumberOfLimbs(4);
    } else {
        setNumberOfLimbs(6);
    }
}

}
