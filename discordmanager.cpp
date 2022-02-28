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
      settings_(new QSettings), ready_(false), failed_(false), running_(false), activityInit_(false),
      lobbyId_(-1), peerLobbyId_(-1), peerUserId_(-1), currentPartySize_(1), activityPartySize_(1), activityLobbyId_(-1), isHost_(true),
      activityIsHost_(true), commandRegistered_(false)
{
    qDebug() << "Discord Manager: Initalizing Discord Manager.";
    qDebug() << "Discord Manager: DISCORD_INSTANCE_ID " << qgetenv("DISCORD_INSTANCE_ID");
    partyId_ = QUuid::createUuid().toString();
    QByteArray partyBA = partyId_.toUtf8();
    std::strncpy(partyIdArr_, partyBA.data(), partyId_.size());
    connect(updateTimer_, &QTimer::timeout, this, &DiscordManager::update);
    connect(messageBufferTimer_, &QTimer::timeout, this, &DiscordManager::checkMessageBuffers);
    setup();
    qDebug() << "Discord Manager: Initalized Discord Manager.";
}

DiscordManager::~DiscordManager()
{
    qDebug() << "Discord Manager: Destroying Discord Manager.";
    if (core_) {
        if(lobbyId_ != -1) {
            core_->LobbyManager().DeleteLobby(lobbyId_, [&](discord::Result) {});
        }
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
    messageBufferTimer_->start(DISCORD_MESSAGE_BUFFER_TIMER);
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
}
void DiscordManager::clearActivity()
{
    if(failed_) {
        return;
    }
    qDebug() << "Discord Manager: Clearing Discord activity.";

    currentActivityDetails_ = activityDetails_ = currentActivityState_ = activityState_ = currentActivityImageText_ = activityImageText_ = "";
    currentPartySize_ = activityPartySize_ = 1;
    activityLobbyId_ = -1;
    activityIsHost_ = true;
    core_->ActivityManager().ClearActivity([](discord::Result result) {
        if (result != discord::Result::Ok) {
            qCritical() << "Discord Manager: Activity clear failed";
        }
    });
}
void DiscordManager::updateActivity()
{
//    return;
    if(failed_) {
        return;
    }
    currentActivityDetails_ = activityDetails_;
    currentActivityState_ = activityDetails_;
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
        if (lobbyId_ != -1) {
            activity.GetParty().SetId(lobbyIdStr_);
            activity.GetParty().GetSize().SetCurrentSize(currentPartySize_);
            activity.GetParty().GetSize().SetMaxSize(DISCORD_LOBBY_SIZE);
            activity.GetSecrets().SetJoin(lobbySecret_);
        }
    } else {
        activity.GetParty().SetId(peerLobbyIdStr_);
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
                     << ")\n";
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


    core_->LobbyManager().OnMemberDisconnect.Connect([&](discord::LobbyId lobbyId, discord::UserId userId) {
        qDebug() << "Discord Manager: Lobbdy Disconnect " << lobbyId << userId;
       if(lobbyId == peerLobbyId_ && userId == peerUserId_) {
           qDebug() << "Discord Manager: Emitting lobbyDisconnect";
           emit onLobbyDisconnect();
       }
    });

    core_->LobbyManager().OnMemberConnect.Connect([&](discord::LobbyId lobbyId, discord::UserId userId) {
        qDebug() << "Discord Manager: Lobbdy Connect " << lobbyId << userId;
    });

    core_->LobbyManager().OnLobbyDelete.Connect([&](discord::LobbyId lobbyId, std::uint32_t reason) {
       qDebug() << "Discord Manager: Lobbdy Delete " << lobbyId << reason;
       if(lobbyId == peerLobbyId_) {
           qDebug() << "Discord Manager: Emitting lobbyDisconnect";
           emit onLobbyDisconnect();
       }
    });

    auto userResult = core_->UserManager().GetCurrentUser(currentUser_);
    QString appPath = QCoreApplication::applicationFilePath();
    appPath.replace("/", "\\");
    QByteArray appPathBA = appPath.toUtf8();
    std::strncpy(appCommand, appPathBA.data(), 2048);


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
        auto userResult =core_->UserManager().GetCurrentUser(currentUser_);
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
    core_->LobbyManager().OnLobbyMessage.Connect([&](discord::LobbyId lobbyId, discord::UserId userId,  std::uint8_t* data, std::uint32_t dataLen){
        qDebug() << "Discord Manager: Received Lobby Message" << lobbyId << userId << QString::fromUtf8((char*)data, dataLen);
        if (lobbyId == peerLobbyId_ && userId == peerUserId_) {
            qDebug() << "Discord Manager: Message accepted";
            QJsonDocument jsonMsg = QJsonDocument::fromJson(QString::fromUtf8((char*)data, dataLen).toUtf8());
            QJsonObject obj = jsonMsg.object();
            if (obj.contains("message")) {
                QString message = obj.value("message").toString();
                if (obj.contains("channel")) {
                    QString ch = obj.value("channel").toString();
                    if (ch == "1") {
                        qDebug() << "Discord Manager: Message received on channel 1";
                        emit onMessageFromChannel1(message);
                    } else if (ch == "2") {
                        qDebug() << "Discord Manager: Message received on channel 2";
                        emit onMessageFromChannel2(message);
                    } else if (ch == "3") {
                        qDebug() << "Discord Manager: Message received on channel 3";
                        emit onMessageFromChannel3(message);
                    }
                }
            }
        }
    });
    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {
        qDebug() << "Discord Manager: Creating Lobby";
        discord::LobbyTransaction txn;
        core_->LobbyManager().GetLobbyCreateTransaction(&txn);
        txn.SetType(discord::LobbyType::Public);
        txn.SetCapacity(DISCORD_LOBBY_SIZE);
        qDebug() << "Discord Manager: Created Lobby Transaction";

        core_->LobbyManager().CreateLobby(txn, [&](discord::Result result, const discord::Lobby &lobby) {
            if (result == discord::Result::Ok) {
                qDebug() << "Discord Manager: Lobby created " << lobby.GetId();
                core_->LobbyManager().GetLobbyActivitySecret(lobby.GetId(), lobbySecret_);
                qDebug() << "Discord Manager: Lobby secret " << QString::fromUtf8(lobbySecret_);
                lobbyId_ = lobby.GetId();
                QString idStr = QString::number(lobbyId_);
                QByteArray idStrBA = idStr.toUtf8();
                memset(lobbyIdStr_, 0, 512);
                strncpy(lobbyIdStr_, idStrBA.data(), idStr.size());
                emit onLobbyIdChange(QString::fromUtf8(lobbySecret_));

            } else {
                qCritical() << "Discord Manager: Failed to create lobby" << int(result);
            }
        });

        core_->ActivityManager().OnActivityJoinRequest.Connect([&] (const discord::User &user) {
           if(settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY_JOIN, false).toBool()) {
               if (lobbyId_ != -1) {
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
           qDebug() << "Joining through invitation with secret " << secret;
           emit onInviteAccepted(QString(secret));
        });
        core_->ActivityManager().OnActivityInvite.Connect([&] (discord::ActivityActionType type, const discord::User &user, const discord::Activity &activity) {
            qDebug() << "Received an invitation: " << int(type) << user.GetId() << activity.GetName();
        });
        core_->ActivityManager().OnActivitySpectate.Connect([&] (const char *secret) {
            qDebug() << "OnActivitySpectate " << secret;
        });


    }


}

void DiscordManager::sendMessageToLobby(QString msg)
{
    qDebug() << "Discord Manager: Sending message to lobby.";
    if(lobbyId_ != -1) {
        std::uint8_t *msgPayload = (std::uint8_t*) msg.toUtf8().data();
        qDebug() << "Discord Manager: message payload ready.";
        core_->LobbyManager().SendLobbyMessage(lobbyId_, msgPayload, msg.size(), [&](discord::Result result) {
            if (result != discord::Result::Ok) {
                qCritical() << "Discord Manager: failed to sent message to lobby: " << int (result);
            }
        });
    } else {
        qWarning() << "Discord Manager: Lobby ID not set.";
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

long long DiscordManager::getLobbyId() const
{
    return lobbyId_;
}



bool DiscordManager::running() const
{
    return running_;
}


bool DiscordManager::connectTo(QString lobbySecret)
{
    qDebug() << "Discord Manager: Connecting to " << lobbySecret;
    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {
        qDebug() << "Discord Manager: Parsing lobby secret";
        QByteArray lobbySecretBA = lobbySecret.toUtf8();
        strncpy(peerLobbySecret_, lobbySecretBA.data(), lobbySecret.size());
        auto split = lobbySecret.split(":");
        if (split.size() != 2) {
            qWarning() << "Discord Manager: Invalid secret format";
            return false;
        }
        if (split[0] == QString::number(lobbyId_)) {
            qWarning() << "Discord Manager: Attempting to connect to self?";
            return false;
        }
        qDebug() << "Discord Manager: Establishing lobby connection";
        core_->LobbyManager().ConnectLobbyWithActivitySecret(peerLobbySecret_, [&](discord::Result result, const discord::Lobby &lobby) {
            if (result == discord::Result::Ok) {
                setIsHost(false);
                qDebug() << "Discord Manager: Connected to lobby " << lobby.GetId() << lobby.GetOwnerId();
                peerUserId_ = lobby.GetOwnerId();
                peerLobbyId_ = lobby.GetId();
                QString idStr = QString::number(peerLobbyId_);
                QByteArray idStrBA = idStr.toUtf8();
                memset(peerLobbyIdStr_, 0, 512);
                strncpy(peerLobbyIdStr_, idStrBA.data(), idStr.size());
                qDebug() << "Discord Manager: Emitting connection succeeded";
                emit connectionSucceeded();
            } else {
                setIsHost(true);
                qCritical() << "Discord Manager: Failed to connect to lobby " << int(result);
                emit connectionFailed();
            }

        });

    }

    return true;

}




void DiscordManager::sendMessageOnChannel1(QString msg) {
    qDebug() << "Discord Manager: sendMessageOnChannel1";
    if(lobbyId_ != -1) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "1.0");
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", "1");
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        ch1Buffer = msgJson;
    } else {
        qWarning() << "Discord Manager: Lobby ID not set.";
    }

}

void DiscordManager::sendMessageOnChannel2(QString msg) {
    qDebug() << "Discord Manager: sendMessageOnChannel2";
    if(lobbyId_ != -1) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "1.0");
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", "2");
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        ch2Buffer = msgJson;
    } else {
        qWarning() << "Discord Manager: Lobby ID not set.";
    }
}

void DiscordManager::sendMessageOnChannel3(QString msg) {
    qDebug() << "Discord Manager: sendMessageOnChannel3";
    if(lobbyId_ != -1) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "1.0");
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", "3");
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        ch3Buffer = msgJson;
    } else {
        qWarning() << "Discord Manager: Lobby ID not set.";
    }
}

void DiscordManager::disconnectFromLobby() {
    qDebug() << "Discord Manager: disconnecting from lobby" << peerLobbyId_;
    if (peerLobbyId_ != -1) {
        core_->LobbyManager().DisconnectLobby(peerLobbyId_, [&](discord::Result result) {
            setIsHost(true);
            if (result != discord::Result::Ok) {
                qWarning() << "Discord Manager: Disconnect failed " << int(result);
            } else {

                qDebug() << "Discord Manager: Disconnected from lobby " << peerLobbyId_;
            }
        });
        peerLobbyId_ = -1;
        peerUserId_ = -1;
    }

}

void DiscordManager::checkMessageBuffers() {
    QMutexLocker lock(&bufferMutex_);
    if (ch1Buffer.size()) {
        QString payload = ch1Buffer;
        ch1Buffer = "";
        sendMessageToLobby(payload);
    }
    if (ch2Buffer.size()) {
        QString payload = ch2Buffer;
        ch2Buffer = "";
        sendMessageToLobby(payload);
    }
    if (ch3Buffer.size()) {
        QString payload = ch3Buffer;
        ch3Buffer = "";
        sendMessageToLobby(payload);
    }
    std::int32_t count;
    if (peerLobbyId_ != -1) {
        auto result = core_->LobbyManager().MemberCount(peerLobbyId_, &count);
        if (result == discord::Result::Ok) {
            currentPartySize_ = count;
        } else {
            qWarning () << "Failed to get member count" << int(result);
        }
    } else if (lobbyId_ != -1) {
        auto result = core_->LobbyManager().MemberCount(lobbyId_, &count);
        if (result == discord::Result::Ok) {
            currentPartySize_ = count;
        } else {
            qWarning () << "Failed to get member count" << int(result);
        }
    } else {
        currentPartySize_ = 1;
    }

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
