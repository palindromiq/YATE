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

int HuntInfo::nightCount() const
{
    return nights_.size();
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
        if (eido == Eidolon::Teralyst) {
            return ANALYSIS_STAT_TERALYST;
        } else if (eido == Eidolon::Gantulyst) {
            return ANALYSIS_STAT_GANTULYST;
        } else if (eido == Eidolon::Hydrolyst) {
           return ANALYSIS_STAT_HYDROLYST;
        }
    } else {
        if (eido == Eidolon::Teralyst) {
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
        return HuntInfo::eidolonName(Eidolon::Teralyst, abbreviate);
    } else if (eido == 1) {
        return HuntInfo::eidolonName(Eidolon::Gantulyst, abbreviate);
    } else if (eido == 2) {
        return HuntInfo::eidolonName(Eidolon::Hydrolyst, abbreviate);
    }
    return "N/A";
}

QString HuntInfo::timestampToProgressString(float timestamp)
{
    if (timestamp < 0.0) {
        return "N/A";
    }
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


void NightInfo::addSquadMember(const QString &member)
{
    if (member != host_) {
        squad_.insert(member);
    }
}

void NightInfo::addSquadMembers(const QSet<QString> &members) {
    for(auto &mem: members) {
        if (mem != host_) {
            squad_.insert(mem);
        }
    }
}

void NightInfo::removeSquadMember(const QString &member)
{
    squad_.remove(member);
}


QString NightInfo::squadString() const
{
    QStringList sq;
    if(host_.size()) {
        sq.push_back(host_);
    }
    for(auto &s: squad_) {
        sq.push_back(s);
    }
    return sq.join(", ");
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
    QString nightRes = getNightResult();
    if (getNumberOfHydrolysts()) {
        nightRes = nightRes + " (Average: " + getAverage() + ")";
    }
    AnalysisViewItem *nightItem = new AnalysisViewItem({ANALYSIS_STAT_NIGHT_NO + QString::number(nightNo), nightRes});
    auto nightRuns = runs();
    for (int i = 0; i < nightRuns.size(); i++) {
        if (nightRuns[i].getNumberOfCaps() || nightRuns[i].getNumberOfKills()) {
            nightItem->appendChild(nightRuns[i].toAnalysisViewItem(i + 1));
        }
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

QString NightInfo::getAverage() const
{
    int count = 0;
    float total = 0;
    auto nightRuns = runs();
    for (auto &r: nightRuns) {
        int numCaps = r.getNumberOfCaps();
        if (numCaps == 3) {
            count++;
            total += r.hydrolystCapInfo().lastLimbProgressTime();
        }
    }
    if (count == 0 || total == 0.0) {
        return "N/A";
    }
    return HuntInfo::timestampToProgressString(total*1.0/count);

}

int NightInfo::getNumberOfHydrolysts() const
{
    int x3s = 0;
    auto nightRuns = runs();
    for (auto &r: nightRuns) {
        int numCaps = r.getNumberOfCaps();
        if (numCaps == 3) {
            x3s++;
        }
    }
    return x3s;
}

int NightInfo::validRunCount() const
{
    int count = 0;
    auto nightRuns = runs();
    for (int i = 0; i < nightRuns.size(); i++) {
        if (nightRuns[i].getNumberOfCaps() || nightRuns[i].getNumberOfKills()) {
            count++;
        }
    }
    return count;
}

float NightInfo::startTimestamp() const
{
    return startTimestamp_;
}

void NightInfo::setStartTimestamp(float newStartTime)
{
    startTimestamp_ = newStartTime;
}

const QString &NightInfo::host() const
{
    return host_;
}

void NightInfo::setHost(const QString &newHost)
{
    host_ = newHost;
}

const QSet<QString> &NightInfo::squad() const
{
    return squad_;
}

void NightInfo::setSquad(const QSet<QString> &newSquad)
{
    squad_ = newSquad;
}

RunInfo::RunInfo(): hasLoadTime_(false)
{

}

CapInfo &RunInfo::teralystCapInfo()
{
    return teralystCapInfo_;
}

void RunInfo::setTeralystCapInfo(const CapInfo &newTeralystCapInfo)
{
    teralystCapInfo_ = newTeralystCapInfo;
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
        return teralystCapInfo();
    }
}

void RunInfo::clear()
{
    CapInfo emptyCap;
    setTeralystCapInfo(emptyCap);
    setGantulystCapInfo(emptyCap);
    setHydrolystCapInfo(emptyCap);
}

AnalysisViewItem *RunInfo::toAnalysisViewItem(int runNo)
{
    QString runRes = getRunResult();
    if(getNumberOfCaps() == 3) {
        runRes = runRes + " (" + HuntInfo::timestampToProgressString(hydrolystCapInfo().lastLimbProgressTime()) + ")";
    }
    AnalysisViewItem *runItem = new AnalysisViewItem({ANALYSIS_STAT_RUN_NO + QString::number(runNo), runRes});
    if(teralystCapInfo().valid()) {
        QString missionLoadTime;
        if (hasLoadTime()) {
            missionLoadTime = FORMAT_NUMBER(loadTime());
        }
        runItem->appendChild(teralystCapInfo().toAnalysisViewItem(missionLoadTime));
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
    int caps = getNumberOfCaps();
    if (caps) {
        return "1x" + QString::number(getNumberOfCaps());
    } else {
        return "0x0";
    }

}

int RunInfo::getNumberOfCaps()
{
    int caps = 0;
    if(teralystCapInfo().valid() && teralystCapInfo().result() == CapState::Capture) {
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

int RunInfo::getNumberOfKills()
{
    int caps = 0;
    if(teralystCapInfo().valid() && teralystCapInfo().result() == CapState::Kill) {
        caps++;
    }
    if(gantulystCapInfo().valid() && gantulystCapInfo().result() == CapState::Kill) {
        caps++;
    }
    if(hydrolystCapInfo().valid() && hydrolystCapInfo().result() == CapState::Kill) {
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

const QVector<LogEvent> &RunInfo::eventLog() const
{
    return eventLog_;
}

void RunInfo::addEvent(const LogEvent &evt) {
    eventLog_.push_back(evt);
}

float RunInfo::loadTime() const
{
    return loadTime_;
}

void RunInfo::setLoadTime(float newLoadTime)
{
    hasLoadTime_ = true;
    loadTime_ = newLoadTime;
}

bool RunInfo::hasLoadTime() const
{
    return hasLoadTime_;
}

CapInfo::CapInfo():valid_(false), shrineTime_(0), lateShardInsertLog_(false)
{
    spawnDelay_ = 0;
    waterShield_ = 0;
    shrineActivationTime_ = 0;
    capshotTime_ = 0;
    lastLimbProgessTime_ = 0;
    loadTime_ = 0;
    lootDropTime_ = 0;
    lootDropTimestamp_ = 0;
    spawnTimestamp_ = 0;
    capshotProgressTimestamp_ = 0;
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
    if (eidolon_ == Eidolon::Teralyst) {
        setNumberOfLimbs(4);
    } else {
        setNumberOfLimbs(6);
    }
}

AnalysisViewItem *CapInfo::toAnalysisViewItem(QString firstLoadTime) const
{
    QString eidolonName = "N/A";
    if (eidolon() == Eidolon::Teralyst) {
        eidolonName = ANALYSIS_STAT_TERALYST;

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
    if (firstLoadTime.size()) {
        capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LOADTIME, firstLoadTime}));
    }

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
            if(eidolon() != Eidolon::Teralyst) {
                ws = ws + " (";
                if (spawnDelay() >= 0) {
                    ws += "+";
                }
                ws = ws + FORMAT_NUMBER(spawnDelay());
                ws = ws + " = ";
                ws = ws + FORMAT_NUMBER(spawnDelay() + waterShield());
                ws = ws + ")";
            }
          }
        capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_WATERSHIELD, ws}));
        QStringList limbsList;
        for (auto &l: limbBreaks()) {
            limbsList.push_back(FORMAT_NUMBER(l));
        }
        capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LIMBS, limbsList.join("  ")}));
        if (numberOfLimbs() == limbBreaks().size()) {
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LAST_LIMB, FORMAT_NUMBER(lastLimbProgressTime())}));
        }
        if (result() != CapState::Spawned) {
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_CAPSHOT, FORMAT_NUMBER(capshotTime())}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_CAPSHOT_TIME, HuntInfo::timestampToProgressString(capshotProgressTimestamp())}));

            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LOOT_DROP, FORMAT_NUMBER(lootDropTime())}));
            float limbsAvg = 0;
            for (auto &l: limbBreaks()) {
                limbsAvg += l;
            }
            limbsAvg = limbsAvg / limbBreaks().size();
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LIMBS_AVERAGE, FORMAT_NUMBER(limbsAvg)}));
            capItem->appendChild(new AnalysisViewItem({ANALYSIS_STAT_LAST_LIMB, HuntInfo::timestampToProgressString(lastLimbProgressTime())}));
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

bool CapInfo::lateShardInsertLog() const
{
    return lateShardInsertLog_;
}

void CapInfo::setLateShardInsertLog(bool newLateShardInsertLog)
{
    lateShardInsertLog_ = newLateShardInsertLog;
}



}
