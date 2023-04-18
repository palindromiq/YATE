#include "discordmanager.h"
#ifdef DISCORD_ENABLED

#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QSettings>
#include <QMutexLocker>
#include "discord_game_sdk/discord.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QApplication>


#include "updater.h"

namespace Yate {
DiscordManager::DiscordManager(QObject *parent)
    : QObject{parent}, currentUser_(new discord::User), updateTimer_(new QTimer(this)), messageBufferTimer_(new QTimer(this)),
      settings_(new QSettings), ready_(false), failed_(false), running_(false), activityInit_(false), currentPartySize_(1), activityPartySize_(1), isHost_(true),
      activityIsHost_(true), commandRegistered_(false)
{
    qDebug() << "Discord Manager: Initalizing Discord Manager.";
    qDebug() << "Discord Manager: DISCORD_INSTANCE_ID " << qgetenv("DISCORD_INSTANCE_ID");
    connect(updateTimer_, &QTimer::timeout, this, &DiscordManager::update);
    setup();
    qDebug() << "Discord Manager: Initalized Discord Manager.";
}

DiscordManager::~DiscordManager()
{
    qDebug() << "Discord Manager: Destroying Discord Manager.";
    if (core_) {
        qDebug() << "Discord Manager: Deleting core";
        delete core_;
    }
}

void DiscordManager::start()
{
    qDebug() << "Discord Manager: Starting.";
    if (running()) {
        qDebug () << "Discord Manager: Already running!";
        return;
    }
    qDebug() << "Discord Manager: Starting timers.";
    updateTimer_->start(DISCORD_UPDATE_TIMER);
    running_ = true;
}

void DiscordManager::stop()
{
    qDebug() << "Discord Manager: Stopping.";
    updateTimer_->stop();
    messageBufferTimer_->stop();
    running_ = false;
}

void DiscordManager::update()
{
    if (failed_) {
        setup(false);
        if (failed_) {
            return;
        }
    }
    core_->RunCallbacks();
    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        if(!activityInit_ || currentActivityDetails_ != activityDetails_ || currentActivityState_ != activityState_
                || currentActivityImageText_ != activityImageText_ || currentPartySize_ != activityPartySize_ || lobbyId_ != activityLobbyId_
                || activityIsHost_ != isHost()) {
            updateActivity();
        }
    }
    core_->NetworkManager().Flush();
    core_->LobbyManager().FlushNetwork();

}
void DiscordManager::clearActivity()
{
    if(failed_) {
        return;
    }
    qDebug() << "Discord Manager: Clearing Discord activity.";

    currentActivityDetails_ = activityDetails_ = currentActivityState_ = activityState_ = currentActivityImageText_ = activityImageText_ = "";
    currentPartySize_ = activityPartySize_ = 1;
    activityLobbyId_ = "";
    activityIsHost_ = true;
    core_->ActivityManager().ClearActivity([](discord::Result result) {
        if (result != discord::Result::Ok) {
            qCritical() << "Discord Manager: Activity clear failed";
        }
    });
}
void DiscordManager::updateActivity()
{
    if(failed_) {
        return;
    }

    currentActivityDetails_ = activityDetails_;
    currentActivityState_ = activityState_;
    qDebug() << currentActivityState_ << " " << activityState_;
    currentActivityImageText_ = activityImageText_;
    currentPartySize_ = activityPartySize_;
    activityLobbyId_ = lobbyId_;
    activityIsHost_ = isHost();
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
    activity.SetState(activityState_.size()? stateArr: "  ");


    if (isHost()) {
        if (lobbyId_ != "") {
            activity.GetParty().SetId(lobbyUsername_.toUtf8().constData());
            activity.GetParty().GetSize().SetCurrentSize(currentPartySize_);
            activity.GetParty().GetSize().SetMaxSize(DISCORD_LOBBY_SIZE);
            qDebug() << "SEtting join sercret to" << lobbyId_.toUtf8().constData();
            activity.GetSecrets().SetJoin(lobbyId_.toUtf8().constData());
        }
    } else {
        activity.GetParty().SetId(peerLobbyId_.toUtf8().constData());
        activity.GetParty().GetSize().SetCurrentSize(currentPartySize_);
        activity.GetParty().GetSize().SetMaxSize(DISCORD_LOBBY_SIZE);
    }

    core_->ActivityManager().UpdateActivity(activity, [&](discord::Result result) {
        if (result != discord::Result::Ok) {
            qWarning() << "Discord Manager: Activity update failed" << int(result);

        }
    });


    activityInit_ = true;
}

void DiscordManager::setup(bool emitErrors)
{

    auto result = discord::Core::Create(DISCORD_CLIENT_ID, DiscordCreateFlags_NoRequireDiscord, &core_);

    if (!core_) {
        if (emitErrors) {
            qCritical() << "Discord Manager: Failed to instantiate discord core! (err " << static_cast<int>(result)
                     << ")";
            emit failed("Failed to connect to Discord");
        }
        failed_ = true;
        return;
    }
    qDebug() << "Discord Manager: Core initialized.";
    failed_ = false;


    core_->SetLogHook(
                discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
        if (level == discord::LogLevel::Debug) {
            qDebug() << "Discord Manager: LogHook(" << static_cast<uint32_t>(level) << "): " << message;
        } else if (level == discord::LogLevel::Info) {
            qDebug() << "Discord Manager: LogHook(" << static_cast<uint32_t>(level) << "): " << message;
        } else if (level == discord::LogLevel::Warn) {
            qWarning() << "Discord Manager: LogHook(" << static_cast<uint32_t>(level) << "): " << message;
        } else {
            qCritical() << "Discord Manager: LogHook(" << static_cast<uint32_t>(level) << "): " << message;
        }

    });


    DiscordNetworkPeerId peerId;
    core_->NetworkManager().GetPeerId(&peerId);
    emit onPeerIdChange(QString::number(peerId));



    auto userResult = core_->UserManager().GetCurrentUser(currentUser_);
    QString appPath = QCoreApplication::applicationFilePath();
    appPath.replace("/", "\\");
    QByteArray appPathBA = appPath.toUtf8();
    std::memset(appCommand, 0, 2048);
    std::strncpy(appCommand, appPathBA.data(), strlen(appPathBA.data()));

    if (userResult == discord::Result::Ok) {
        qDebug() << "Discord Manager: Got Current User";
        core_->UserManager().GetCurrentUser(currentUser_);
        QString username(currentUser_->GetUsername());
        qDebug() << "Discord Manager: Emitting username" << username;
        emit onUserConnected(username);
        ready_ = true;
    }


    core_->UserManager().OnCurrentUserUpdate.Connect([&]() {
        qDebug() << "Discord Manager: Current User Update";
        auto userResult = core_->UserManager().GetCurrentUser(currentUser_);
        if (userResult == discord::Result::Ok) {
            QString username(currentUser_->GetUsername());
            qDebug() << "Discord Manager: Emitting username" << username;
            emit onUserConnected(username);
            if (!commandRegistered_) {
                auto registerResult = core_->ActivityManager().RegisterCommand(appCommand);

                if (registerResult != discord::Result::Ok) {
                    qWarning() << "Discord Manager: Failed to register result " << int(registerResult);

                } else {
                    qDebug() << "Discord Manager: Registered app command: " << strlen(appCommand);
                    commandRegistered_ = true;
                }
            }
            ready_ = true;

        } else {
             qWarning() << "Discord Manager: Current User Update failed " << int(userResult);
        }


    });

    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_NETWORKING, true).toBool()) {
         core_->ActivityManager().OnActivityJoinRequest.Connect([&] (const discord::User &user) {
           if(settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY_JOIN, false).toBool()) {
               if (lobbyId_ != "") {
                   core_->ActivityManager().SendRequestReply(user.GetId(), discord::ActivityJoinRequestReply::Yes, [&](discord::Result result) {
                        if (result == discord::Result::Ok) {
                            qDebug() << "Discord Manager: Accepted join request";
                        } else {
                            qWarning() << "Discord Manager: Failed to accept join request: " << int(result);
                        }
                   });
               } else {
                   core_->ActivityManager().SendRequestReply(user.GetId(), discord::ActivityJoinRequestReply::Ignore, [&](discord::Result result) {
                       if (result == discord::Result::Ok) {
                           qDebug() << "Discord Manager: Ignored join request";
                       } else {
                           qWarning() << "Discord Manager: Failed to ignore join request: " << int(result);
                       }
                   });
               }
           } else {
               core_->ActivityManager().SendRequestReply(user.GetId(), discord::ActivityJoinRequestReply::No, [&](discord::Result result) {
                   if (result == discord::Result::Ok) {
                       qDebug() << "Discord Manager: Rejected join request";
                   } else {
                       qWarning() << "Discord Manager: Failed to reject join request: " << int(result);
                   }
               });
           }
        });
        core_->ActivityManager().OnActivityJoin.Connect([&] (const char *secret) {
           qDebug() << "Discord Manager: Joining through invitation with secret " << secret;
           emit onInviteAccepted(QString(secret));
        });
        core_->ActivityManager().OnActivityInvite.Connect([&] (discord::ActivityActionType type, const discord::User &user, const discord::Activity &activity) {
            qDebug() << "Discord Manager: Received an invitation: " << int(type) << user.GetId() << activity.GetName();
        });
        core_->ActivityManager().OnActivitySpectate.Connect([&] (const char *secret) {
            qDebug() << "Discord Manager: OnActivitySpectate " << secret;
        });


    }


}

const QString &DiscordManager::getPeerLobbyId() const
{
    return peerLobbyId_;
}

void DiscordManager::setPeerLobbyId(const QString &newPeerLobbyId)
{
    peerLobbyId_ = newPeerLobbyId;
}

const QString &DiscordManager::getLobbyId() const
{
    return lobbyId_;
}

void DiscordManager::setLobbyId(const QString &newLobbyId)
{
    lobbyId_ = newLobbyId;
    auto lobbyParts = lobbyId_.split(":");
    if (lobbyParts.size() == 2) {
        lobbyUsername_ = lobbyParts[0];
    } else {
        lobbyUsername_ = lobbyId_;
        qWarning() << "Discord Manager: invalid lobby ID:" << lobbyId_;
    }
}

bool DiscordManager::isHost() const
{
    return isHost_;
}

void DiscordManager::setIsHost(bool newIsHost)
{
    isHost_ = newIsHost;
}


bool DiscordManager::running() const
{
    return running_;
}


void DiscordManager::setSquadString(QString msg) {
    if (msg.size()) {
        activityImageText_ = msg;
    } else {
        activityImageText_ = "";
    }
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
