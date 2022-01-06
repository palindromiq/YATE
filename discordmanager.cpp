#include "discordmanager.h"
#ifdef DISCORD_ENABLED

#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QSettings>
#include "discord_game_sdk/discord.h"

namespace Yate {
DiscordManager::DiscordManager(QObject *parent)
    : QObject{parent}, currentUser_(new discord::User), updateTimer_(new QTimer(this)), settings_(new QSettings), ready_(false), failed_(false), running_(false), activityInit_(false)
{
    connect(updateTimer_, &QTimer::timeout, this, &DiscordManager::update);
    setup();
}

DiscordManager::~DiscordManager()
{
    delete core_;
    delete currentUser_;
}

void DiscordManager::start()
{
    if (running()) {
        return;
    }
    updateTimer_->start(DISCORD_UPDATE_TIMER);
    running_ = true;
}

void DiscordManager::stop()
{
    updateTimer_->stop();
    running_ = false;
}

void DiscordManager::update()
{
    if (failed_) {
        setup(false);
        return;
    }
    core_->RunCallbacks();
    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        if(!activityInit_ || currentActivityDetails_ != activityDetails_ || currentActivityState_ != activityState_ || currentActivityImageText_ != activityImageText_) {
            updateActivity();
        }
    }
}
void DiscordManager::clearActivity()
{
    if(failed_) {
        return;
    }
    currentActivityDetails_ = activityDetails_ = currentActivityState_ = activityState_ = currentActivityImageText_ = activityImageText_ = "";
    core_->ActivityManager().ClearActivity([](discord::Result result) {
        if (result != discord::Result::Ok) {
            qDebug() << "Activity clear failed";
        }
    });
}
void DiscordManager::updateActivity()
{
    if(failed_) {
        return;
    }
    currentActivityDetails_ = activityDetails_;
    currentActivityState_ = activityDetails_;
    currentActivityImageText_ = activityImageText_;
    discord::Activity activity{};
    QByteArray detailsBA = activityDetails_.toUtf8();
    const char *detailsArr = detailsBA.data();
    QByteArray stateBA = activityState_.toUtf8();
    const char *stateArr = stateBA.data();
    QByteArray imageTextBA = activityImageText_.toUtf8();
    const char *imageTextArr = imageTextBA.data();
    activity.SetDetails(detailsArr);
    activity.GetAssets().SetLargeImage("logo");
    activity.GetAssets().SetLargeText(imageTextArr);
    activity.SetState(stateArr);
    core_->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result != discord::Result::Ok) {
            qDebug() << "Activity update failed";
        }
    });
    activityInit_ = true;
}

void DiscordManager::setup(bool emitErrors)
{

    auto result = discord::Core::Create(DISCORD_CLIENT_ID, DiscordCreateFlags_NoRequireDiscord, &core_);


    if (!core_) {
        if (emitErrors) {
            qDebug() << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                     << ")\n";
            emit failed("Failed to connect to Discord");
        }
        failed_ = true;
        return;
    }
    failed_ = false;


    core_->SetLogHook(
                discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
        qDebug() << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
    });

    core_->UserManager().OnCurrentUserUpdate.Connect([&]() {
        core_->UserManager().GetCurrentUser(currentUser_);
        ready_ = true;
        emit ready();

    });


}

bool DiscordManager::running() const
{
    return running_;
}

const QSet<QString> &DiscordManager::squad() const
{
    return squad_;
}

void DiscordManager::setSquad(const QSet<QString> &newSquad)
{
    squad_ = newSquad;
    if (squad_.size()) {
        activityImageText_ = tr("Hunting with ") + QStringList(squad_.begin(), squad_.end()).join(", ");
    } else {
        activityImageText_ = tr("Hunting Solo");
    }

}

const QString &DiscordManager::host() const
{
    return host_;
}

void DiscordManager::setHost(const QString &newHost)
{
    host_ = newHost;
}

const QString &DiscordManager::activityState() const
{
    return activityState_;
}

void DiscordManager::setActivityState(const QString &newActivityState)
{
    activityState_ = newActivityState;
}

const QString &DiscordManager::activityDetails() const
{
    return activityDetails_;
}

void DiscordManager::setActivityDetails(const QString &newActivityDetails)
{
    activityDetails_ = newActivityDetails;
}

}

#endif
