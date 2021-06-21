#include "huntinfo.h"
#include "analysisviewitem.h"




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

NightInfo &HuntInfo::night(int index)
{
    return nights_[index];
}

void HuntInfo::removeNight(int index)
{
    nights_.removeAt(index);
}

void HuntInfo::clear()
{
    nights_.clear();
}

QString HuntInfo::eidolonName(Eidolon eido, bool abbreviate)
{
    if (!abbreviate) {
        if (eido == Eidolon::Terralyst) {
            return ANALYSIS_STAT_TERRALYST;
        } else if (eido == Eidolon::Gantulyst) {
            return ANALYSIS_STAT_GANTULYST;
        } else if (eido == Eidolon::Hydrolyst) {
           return ANALYSIS_STAT_HYDROLYST;
        }
    } else {
        if (eido == Eidolon::Terralyst) {
            return ANALYSIS_STAT_TERRY;
        } else if (eido == Eidolon::Gantulyst) {
            return ANALYSIS_STAT_GARRY;
        } else if (eido == Eidolon::Hydrolyst) {
           return ANALYSIS_STAT_HARRY;
        }
    }
    return "N/A";
}

QString HuntInfo::eidolonName(int eido, bool abbreviate)
{
    if (eido == 0) {
        return HuntInfo::eidolonName(Eidolon::Terralyst, abbreviate);
    } else if (eido == 1) {
        return HuntInfo::eidolonName(Eidolon::Gantulyst, abbreviate);
    } else if (eido == 2) {
        return HuntInfo::eidolonName(Eidolon::Hydrolyst, abbreviate);
    }
    return "N/A";
}

QString HuntInfo::timestampToProgressString(float timestamp)
{
    int tsMins = int(timestamp / 60.0);
    return QString::number(tsMins) + ":" + FORMAT_NUMBER(timestamp - (tsMins * 60));

}


QVector<AnalysisViewItem *> HuntInfo::toAnalysisViewItem() const
{
    //TODO: GC
    QVector<AnalysisViewItem *> items;
    auto infoNights = nights();
    for(int i = 0; i < infoNights.size(); i++) {
        AnalysisViewItem* nightItem = infoNights[i].toAnalysisViewItem(i + 1);
        items.push_back(nightItem);
    }
    return items;
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

RunInfo &NightInfo::run(int index)
{
    return runs_[index];
}

void NightInfo::removeRun(int index)
{
    runs_.removeAt(index);
}

void NightInfo::clear()
{
    runs_.clear();
}

AnalysisViewItem *NightInfo::toAnalysisViewItem(int nightNo) const
{
    AnalysisViewItem *nightItem = new AnalysisViewItem({ANALYSIS_STAT_NIGHT_NO + QString::number(nightNo), getNightResult()});
    auto nightRuns = runs();
    for (int i = 0; i < nightRuns.size(); i++) {
        nightItem->appendChild(nightRuns[i].toAnalysisViewItem(i + 1));
    }
    return nightItem;

}

QString NightInfo::getNightResult() const
{
    int x3s = 0;
    int x2s = 0;
    int x1s = 0;
    QStringList results;
    auto nightRuns = runs();
    for (auto &r: nightRuns) {
        int numCaps = r.getNumberOfCaps();
        if (numCaps == 3) {
            x3s++;
        } else if (numCaps == 2) {
            x2s++;
        } else if (numCaps == 1) {
            x1s++;
        }
    }
    if(x3s) {
        results.append(QString::number(x3s) + "x3");
    }
    if(x2s) {
        results.append(QString::number(x2s) + "x2");
    }
    if(x1s) {
        results.append(QString::number(x1s) + "x1");
    }
    return results.join("+");
}

float NightInfo::startTimestamp() const
{
    return startTimestamp_;
}

void NightInfo::setStartTimestamp(float newStartTime)
{
    startTimestamp_ = newStartTime;
}

RunInfo::RunInfo()
{

}

CapInfo &RunInfo::terralystCapInfo()
{
    return terralystCapInfo_;
}

void RunInfo::setTerralystCapInfo(const CapInfo &newTerralystCapInfo)
{
    terralystCapInfo_ = newTerralystCapInfo;
}

CapInfo &RunInfo::gantulystCapInfo()
{
    return gantulystCapInfo_;
}

void RunInfo::setGantulystCapInfo(const CapInfo &newGantulystCapInfo)
{
    gantulystCapInfo_ = newGantulystCapInfo;
}

CapInfo &RunInfo::hydrolystCapInfo()
{
    return hydrolystCapInfo_;
}

void RunInfo::setHydrolystCapInfo(const CapInfo &newHydrolystCapInfo)
{
    hydrolystCapInfo_ = newHydrolystCapInfo;
}

CapInfo &RunInfo::capInfoByIndex(int index)
{
    if (index == 1) {
        return gantulystCapInfo();
    } else if (index == 2) {
        return hydrolystCapInfo();
    } else {
        return terralystCapInfo();
    }
}

void RunInfo::clear()
{
    CapInfo emptyCap;
    setTerralystCapInfo(emptyCap);
    setGantulystCapInfo(emptyCap);
    setHydrolystCapInfo(emptyCap);
}

AnalysisViewItem *RunInfo::toAnalysisViewItem(int runNo)
{
    AnalysisViewItem *runItem = new AnalysisViewItem({ANALYSIS_STAT_RUN_NO + QString::number(runNo), getRunResult()});
    if(terralystCapInfo().valid()) {
        runItem->appendChild(terralystCapInfo().toAnalysisViewItem());
    }
    if(gantulystCapInfo().valid()) {
        runItem->appendChild(gantulystCapInfo().toAnalysisViewItem());
    }
    if(hydrolystCapInfo().valid()) {
        runItem->appendChild(hydrolystCapInfo().toAnalysisViewItem());
    }
    return runItem;
}

QString RunInfo::getRunResult()
{
    return "1x" + QString::number(getNumberOfCaps());
}

int RunInfo::getNumberOfCaps()
{
    int caps = 0;
    if(terralystCapInfo().valid() && terralystCapInfo().result() == CapState::Capture) {
        caps++;
    }
    if(gantulystCapInfo().valid() && gantulystCapInfo().result() == CapState::Capture) {
        caps++;
    }
    if(hydrolystCapInfo().valid() && hydrolystCapInfo().result() == CapState::Capture) {
        caps++;
    }
    return caps;
}

float RunInfo::startTimestamp() const
{
    return startTimestamp_;
}

void RunInfo::setStartTimestamp(float newStartTime)
{
    startTimestamp_ = newStartTime;
}

CapInfo::CapInfo():valid_(false), shrineTime_(0)
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

const QVector<float> &CapInfo::limbBreaks() const
{
    return limbBreaks_;
}

void CapInfo::setlimbBreaks(const QVector<float> &newLimbBreaks)
{
    limbBreaks_ = newLimbBreaks;
}

void CapInfo::addlimbBreak(float &newLimbBreak)
{
    limbBreaks_.push_back(newLimbBreak);
}

float CapInfo::limbBreak(int index) const
{
    return limbBreaks_.at(index);
}

void CapInfo::clearLimbBreaks()
{
    limbBreaks_.clear();
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

AnalysisViewItem *CapInfo::toAnalysisViewItem() const
{
    QString eidolonName = "N/A";
    if (eidolon() == Eidolon::Terralyst) {
        eidolonName = ANALYSIS_STAT_TERRALYST;
    } else if (eidolon() == Eidolon::Gantulyst) {
         eidolonName = ANALYSIS_STAT_GANTULYST;
    } else if (eidolon() == Eidolon::Hydrolyst) {
        eidolonName = ANALYSIS_STAT_HYDROLYST;
    }
    QString resultName = "N/A";
    if (result() == CapState::Capture) {
        resultName = ANALYSIS_STAT_RESULT_CAPTURED;
    } else if (result() == CapState::Kill) {
        resultName = ANALYSIS_STAT_RESULT_KILLED;
    } else if (result() == CapState::InComplete) {
        resultName = ANALYSIS_STAT_RESULT_INCOMPLETE;
    } else if (result() == CapState::Spawned) {
        resultName = ANALYSIS_STAT_RESULT_SPAWNED;
    }
    AnalysisViewItem *capItem = new AnalysisViewItem({eidolonName, resultName});

    if(result() !=  CapState::InComplete) {
        QString ws = FORMAT_NUMBER(waterShield());
        if (shrineTime() > 0) {
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_SHRINE_ACTIVASION, FORMAT_NUMBER(shrineActivationTime())}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_SHRINE_TIME, FORMAT_NUMBER(shrineTime())}));

            auto shardTimes = timeBetweenShards();
            if (shardTimes.length()) {
                QString shardTimeStr;
                for(int i = 0; i < shardTimes.length(); i++) {
                    shardTimeStr = shardTimeStr + FORMAT_NUMBER(shardTimes[i]);
                    if (i != shardTimes.length() - 1) {
                        shardTimeStr = shardTimeStr + ", ";
                    }
                }
                capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_BETWEEN_SHARDS, shardTimeStr}));
            }
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_SPAWN_DELAY, FORMAT_NUMBER(spawnDelay())}));
            if(eidolon() != Eidolon::Terralyst) {
                ws = ws + " (+";
                ws = ws + FORMAT_NUMBER(spawnDelay());
                ws = ws + " = ";
                ws = ws + FORMAT_NUMBER(spawnDelay() + waterShield());
                ws = ws + ")";
            }
          }
        capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_WATERSHIELD, ws}));
        if (numberOfLimbs() == limbBreaks().size()) {
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LAST_LIMB, FORMAT_NUMBER(lastLimbProgressTime())}));
        }
        if (result() != CapState::Spawned) {
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_CAPSHOT, FORMAT_NUMBER(capshotTime())}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LOOT_DROP, FORMAT_NUMBER(lootDropTime())}));
            float limbsAvg = 0;
            for (auto &l: limbBreaks()) {
                limbsAvg += l;
            }
            limbsAvg = limbsAvg / limbBreaks().size();
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LIMBS_AVERAGE, FORMAT_NUMBER(limbsAvg)}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LAST_LIMB, HuntInfo::timestampToProgressString(lastLimbProgressTime())}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_CAPSHOT_TIME, HuntInfo::timestampToProgressString(capshotProgressTimestamp())}));
        }
    }


    return capItem;

}

float CapInfo::lastLimbProgressTime() const
{
    return lastLimbProgessTime_;
}

void CapInfo::setLastLimbProgressTime(float newLastLimbTime)
{
    lastLimbProgessTime_ = newLastLimbTime;
}

float CapInfo::loadTime() const
{
    return loadTime_;
}

void CapInfo::setLoadTime(float newLoadTime)
{
    loadTime_ = newLoadTime;
}

float CapInfo::lootDropTime() const
{
    return lootDropTime_;
}

void CapInfo::setLootDropTime(float newLootDropTime)
{
    lootDropTime_ = newLootDropTime;
}

float CapInfo::lootDropTimestamp() const
{
    return lootDropTimestamp_;
}

void CapInfo::setLootDropTimestamp(float newLootDropTimestamp)
{
    lootDropTimestamp_ = newLootDropTimestamp;
}

float CapInfo::spawnTimestamp() const
{
    return spawnTimestamp_;
}

void CapInfo::setSpawnTimestamp(float newSpawnTimestamp)
{
    spawnTimestamp_ = newSpawnTimestamp;
}

float CapInfo::capshotProgressTimestamp() const
{
    return capshotProgressTimestamp_;
}

void CapInfo::setCapshotProgressTime(float newCapshotTimestamp)
{
    capshotProgressTimestamp_ = newCapshotTimestamp;
}



}
