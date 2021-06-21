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
    emit onHuntStateChanged(" [Live Feedback will show here]");


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
        if (currentNightIndex_ == -1) {
            currentNightIndex_++;
            huntInfo()->addNight(NightInfo());
            huntInfo()->night(currentNightIndex_).setStartTimestamp(timestamp);
            nightEnded_ = false;

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
        }
        if(timestamp - lastEventTime_ > FIRST_SET_NIGHT_START_THRESHOLD) {
            huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setSpawnTimestamp(timestamp);
            huntInfo()->night(currentNightIndex_).run(currentRunIndex_).setStartTimestamp(timestamp);
        }
        state_.setStage(HuntStateStage::Spawned);
        emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber()) + tr(" spawned"));

    } else if (typ == LogEventType::DayBegin) {
         nightEnded_ = true;
         return;
    } else if (typ == LogEventType::HostUnload) {
        if (currentNightIndex_ != -1 && (timestamp -  huntInfo()->night(currentNightIndex_).startTimestamp()) > MAX_NIGHT_DURATION) {
            nightEnded_ = true;
        }
        state_.setStage(HuntStateStage::Initial);
        state_.setEidolonNumber(0);
        emit onHuntStateChanged(" [Live Feedback will show here]");
    } else if (typ == LogEventType::EidolonDespawn) {
        auto capState = huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).result();
         state_.setStage(HuntStateStage::Initial);
        if (capState == CapState::Spawned) {
            emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber()) + tr(" despawned"));
            state_.setEidolonNumber(0);
        } else if (capState == CapState::Capture) {
            state_.setStage(HuntStateStage::Capped);
        } else if (capState == CapState::Kill) {
            state_.setStage(HuntStateStage::Killed);
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
                    float ws = timestamp - spawnTimestamp;
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setWaterShield(ws);
                    state_.setLimbNumber(0);
                    state_.setStage(HuntStateStage::Limbs);

                    QString statStr = ANALYSIS_STAT_WATERSHIELD + ": "+  FORMAT_NUMBER(ws);
                    if (state_.eidolonNumber() != 0) {
                        float spDelay =   huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).spawnDelay();
                        statStr = statStr + " (+";
                        statStr = statStr + FORMAT_NUMBER(spDelay);
                        statStr = statStr + " = ";
                        statStr = statStr + FORMAT_NUMBER(spDelay + ws);
                        statStr = statStr + ")";
                    }
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + statStr);
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
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber(), true) + tr(" Limb ") + QString::number(state_.limbNumber() + 1) + ": " + FORMAT_NUMBER(limbtime));
                    if (state_.limbNumber() == maxLimbs - 1) {
                        state_.setStage(HuntStateStage::Healing);
                        huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setLastLimbProgressTime(timestamp - huntInfo()->night(currentNightIndex_).run(currentRunIndex_).startTimestamp());
                        emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber(), true) + tr(" Limb ")
                                                + QString::number(state_.limbNumber() + 1) + ": " + FORMAT_NUMBER(limbtime)
                                                + " [" + HuntInfo::timestampToProgressString(huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).lastLimbProgressTime())
                                                + "]");
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
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber(), true) + tr(" Capshot") + ": " + FORMAT_NUMBER(capshot) + " ["
                                             + HuntInfo::timestampToProgressString(huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).capshotProgressTimestamp())
                                             + "]");


                } else if (typ == LogEventType::EidolonKill) {
                    float capshot = timestamp - lastEventTime_ - CAPSHOT_ANIMATION_TIME;
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotTime(capshot);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setResult(CapState::Kill);
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setCapshotProgressTime(timestamp - huntInfo()->night(currentNightIndex_).run(currentRunIndex_).startTimestamp());
                    state_.setStage(HuntStateStage::Killed);
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber(), true) + tr(" Killshot") + ": " + FORMAT_NUMBER(capshot) + " ["
                                             + HuntInfo::timestampToProgressString(huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).capshotProgressTimestamp())
                                             + "]");

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
                    shrineDelay_ = timestamp - lastEventTime_;

                    state_.setStage(HuntStateStage::ShrineEnabled);
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + "Shrine Delay: " + FORMAT_NUMBER(shrineDelay_));
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
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setShrineActivationTime(shrineDelay_);

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
                    huntInfo()->night(currentNightIndex_).run(currentRunIndex_).capInfoByIndex(currentCapIndex_).setShrineTime(lastEventTime_ - previousLootDropTs);

                    state_.setStage(HuntStateStage::Spawned);
                    state_.setEidolonNumber(state_.eidolonNumber() + 1);
                    emit onHuntStateChanged(QString(" [#") + QString::number(currentRunIndex_ + 1) + "] " + HuntInfo::eidolonName(state_.eidolonNumber()) + tr(" spawned"));

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
