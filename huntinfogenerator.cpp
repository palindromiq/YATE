#include "huntinfogenerator.h"
#include "huntinfo.h"
#include "globals.h"

#include <QDebug>

namespace  Yate {

HuntInfoGenerator::HuntInfoGenerator(QObject *parent): QObject(parent), huntInfo_(nullptr)
{
    resetHuntInfo();
}

HuntInfo *HuntInfoGenerator::huntInfo() const
{
    return huntInfo_;
}

void HuntInfoGenerator::resetHuntInfo()
{
    if (huntInfo_) {
        delete huntInfo_;
    }

    huntInfo_ = new HuntInfo;
    state_.reset();
    lastEventTime_ = 0;
    currentCapIndex_ = -1;
    currentRunIndex_ = -1;
    currentNightIndex_ = -1;
    nightEnded_ = false;

}

HuntInfoGenerator::~HuntInfoGenerator()
{
    delete huntInfo_;
}



void HuntInfoGenerator::onLogEvent(LogEvent &e)
{
    QMap<LogEventType, QString> logEvtTypName{
            {LogEventType::NightBegin, "NightBegin"},
            {LogEventType::DayBegin, "DayBegin"},
            {LogEventType::TerralystSpawn, "TerralystSpawn"},
            {LogEventType::LimbBreak, "LimbBreak"},
            {LogEventType::EidolonCapture, "EidolonCapture"},
            {LogEventType::EidolonKill, "EidolonKill"},
            {LogEventType::LootDrop, "LootDrop"},
            {LogEventType::ShrineEnable, "ShrineEnable"},
            {LogEventType::ShardInsert, "ShardInsert"},
            {LogEventType::ShardRemove, "ShardRemove"},
            {LogEventType::ShrineDisable, "ShrineDisable"},
            {LogEventType::EidolonSpawn, "EidolonSpawn"},
            {LogEventType::HostUnload, "HostUnload"},
            {LogEventType::Invalid, "Invalid"}
        };
    QMap<HuntStateStage, QString> stateStageName{
        {HuntStateStage::Initial, "Initial"},
        {HuntStateStage::Spawned, "Spawned"},
        {HuntStateStage::Limbs, "Limbs"},
        {HuntStateStage::Healing, "Healing"},
        {HuntStateStage::Capped, "Capped"},
        {HuntStateStage::Killed, "Killed"},
        {HuntStateStage::LootDropped, "LootDropped"},
        {HuntStateStage::ShrineEnabled, "ShrineEnabled"}
    };

    auto typ = e.type();
    float timestamp = e.timestamp();
    bool invalid = false;

    if (typ == LogEventType::NightBegin) {
        if(timestamp - lastEventTime_ > FIRST_SET_NIGHT_START_THRESHOLD) {
            huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnTimestamp(timestamp);
            huntInfo()->night(currentNightIndex_).run(currentRunIndex_).setStartTimestamp(timestamp);
        }
        state_.setStage(HuntStateStage::Spawned);
    } else if (typ == LogEventType::DayBegin) {
         nightEnded_ = true;
    } else if (typ == LogEventType::HostUnload) {
        if (currentNightIndex_ != -1 && (timestamp -  huntInfo()->night(currentNightIndex_).startTimestamp()) > MAX_NIGHT_DURATION) {
            nightEnded_ = true;
        }
   } else {
        switch (state_.stage()) {
            case HuntStateStage::Initial: {
                if (typ == LogEventType::TerralystSpawn) {
                    if (currentNightIndex_ == -1 || nightEnded_) {
                        currentNightIndex_++;
                        huntInfo()->addNight(NightInfo());
                        huntInfo()->night(currentNightIndex_).setStartTimestamp(timestamp);
                        nightEnded_ = false;
                    }

                    currentRunIndex_++;

                    huntInfo()->night(currentNightIndex_).addRun(RunInfo());

                    currentCapIndex_ = 0;

                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setValid(true);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setEidolon(Eidolon::Terralyst);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnDelay(0.0);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setResult(CapState::Spawned);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnTimestamp(timestamp);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).setStartTimestamp(timestamp);
                    state_.setEidolonNumber(0);
                } else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::Spawned: {
                if (typ == LogEventType::LimbBreak) {
                    float spawnTimestamp = huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).spawnTimestamp();
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setWaterShield(timestamp - spawnTimestamp);
                    state_.setLimbNumber(0);
                    state_.setStage(HuntStateStage::Limbs);
                } else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::Limbs: {
                if (typ == LogEventType::LimbBreak) {
                    int maxLimbs = huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).numberOfLimbs();
                    float limbtime = timestamp - lastEventTime_ - LIMB_BREAK_ANIMATION_TIME;
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).addlimbBreak(limbtime);
                    state_.setLimbNumber(state_.limbNumber() + 1);
                    if (state_.limbNumber() == maxLimbs - 1) {
                        state_.setStage(HuntStateStage::Healing);
                        huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setLastLimbProgressTime(timestamp - huntInfo()->night(currentNightIndex_).run(currentRunIndex_).startTimestamp());
                    }
                } else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::Healing: {
                if (typ == LogEventType::EidolonCapture) {
                    float capshot = timestamp - lastEventTime_ - CAPSHOT_ANIMATION_TIME;
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotTime(capshot);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setResult(CapState::Capture);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotProgressTime(timestamp - huntInfo()->night(currentNightIndex_).run(currentRunIndex_).startTimestamp());
                    state_.setStage(HuntStateStage::Capped);

                } else if (typ == LogEventType::EidolonKill) {
                    float capshot = timestamp - lastEventTime_ - CAPSHOT_ANIMATION_TIME;
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotTime(capshot);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setResult(CapState::Kill);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotProgressTime(timestamp - huntInfo()->night(currentNightIndex_).run(currentRunIndex_).startTimestamp());
                    state_.setStage(HuntStateStage::Killed);
                } else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::Capped:
            case HuntStateStage::Killed: {
                if (typ == LogEventType::LootDrop) {
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setLootDropTimestamp(timestamp);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setLootDropTime(timestamp - lastEventTime_);
                    if (huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).eidolon() == Eidolon::Hydrolyst
                            || state_.stage() == HuntStateStage::Killed) {
                        state_.setStage(HuntStateStage::Initial);
                    } else {
                        state_.setStage(HuntStateStage::LootDropped);

                    }


                } else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::LootDropped: {

                if (typ == LogEventType::ShrineEnable) {
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setShrineActivationTime(timestamp - lastEventTime_);
                    state_.setStage(HuntStateStage::ShrineEnabled);
                }  else {
                    invalid = true;
                }
                break;
            }
            case HuntStateStage::ShrineEnabled: {
                if (typ == LogEventType::ShardInsert) {
                    float delay = timestamp - lastEventTime_;
                    currentCapIndex_++;
                    Eidolon nextEido = Eidolon::Gantulyst;
                    if ( huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_ - 1).eidolon() == Eidolon::Gantulyst) {
                        nextEido = Eidolon::Hydrolyst;
                    }
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setValid(true);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).addTimeBetweenShards(delay);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setEidolon(nextEido);

                    state_.setStage(HuntStateStage::ShardInserted);
                } else {
                    invalid = true;
                }
                break;
            } case HuntStateStage::ShardInserted: {
                float delay = timestamp - lastEventTime_;
                if (typ == LogEventType::ShardInsert) {
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).addTimeBetweenShards(delay);
                } else if (typ == LogEventType::EidolonSpawn) {
                    float previousLootDropTs = huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_ - 1).lootDropTimestamp();
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnDelay(delay);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnTimestamp(timestamp);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setResult(CapState::Spawned);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setShrineTime(timestamp - previousLootDropTs);

                    state_.setStage(HuntStateStage::Spawned);
                    state_.setEidolonNumber(state_.eidolonNumber() + 1);

                } else {
                    invalid = true;
                }
                break;
            }
            default: {
                 qDebug() << "Failing..";
                 invalid = true;
            }
       }
    }
    if (invalid) {
        //qDebug() << "Failed!" << int(typ) << state_.eidolonNumber() << int(state_.stage());
        qDebug () << stateStageName[state_.stage()] << "" << int(typ) << logEvtTypName[typ] << "  " << timestamp;
    }
    lastEventTime_ = timestamp;
    lastEvent_ = e;
}

float HuntInfoGenerator::getLastEventTime() const
{
    return lastEventTime_;
}

void HuntInfoGenerator::setLastEventTime(float newLastEventTime)
{
    lastEventTime_ = newLastEventTime;
}

HuntState::HuntState():limbNumber_(-1)
{

}

HuntStateStage HuntState::stage() const
{
    return stage_;
}

void HuntState::setStage(HuntStateStage newStage)
{
    stage_ = newStage;
}

int HuntState::limbNumber() const
{
    return limbNumber_;
}

void HuntState::setLimbNumber(int newLimbNumber)
{
    limbNumber_ = newLimbNumber;
}

int HuntState::eidolonNumber() const
{
    return eidolonNumber_;
}

void HuntState::setEidolonNumber(int newEidolonNumber)
{
    eidolonNumber_ = newEidolonNumber;
}

void HuntState::reset()
{
    setStage(HuntStateStage::Initial);
    setEidolonNumber(0);
    setLimbNumber(0);
}

}
