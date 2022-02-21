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

#include "updater.h"

namespace Yate {
DiscordManager::DiscordManager(QObject *parent)
    : QObject{parent}, currentUser_(new discord::User), updateTimer_(new QTimer(this)), messageBufferTimer_(new QTimer(this)),
      settings_(new QSettings), ready_(false), failed_(false), running_(false), activityInit_(false),
      lobbyId_(-1), peerLobbyId_(-1), peerUserId_(-1)
{
    qDebug() << "Discord Manager: Initalizing Discord Manager.";
    qDebug() << "Discord Manager: DISCORD_INSTANCE_ID " << qgetenv("DISCORD_INSTANCE_ID");
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
        if(!activityInit_ || currentActivityDetails_ != activityDetails_ || currentActivityState_ != activityState_ || currentActivityImageText_ != activityImageText_) {
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

    if(settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY_JOIN, false).toBool()) {
        if (lobbyId_ != -1) {
            activity.SetInstance(true);
            activity.GetParty().SetId(lobbySecret_);
            activity.GetParty().GetSize().SetCurrentSize(1);
            activity.GetParty().GetSize().SetMaxSize(DISCORD_LOBBY_SIZE);
            activity.GetSecrets().SetJoin(lobbySecret_);
        }
    }

    core_->ActivityManager().UpdateActivity(activity, [&](discord::Result result) {
        if (result != discord::Result::Ok) {
            qWarning() << "Discord Manager: Activity update failed";

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
        qDebug() << "Discord Manager: LogHook(" << static_cast<uint32_t>(level) << "): " << message;
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
    if (userResult == discord::Result::Ok) {
        qDebug() << "Discord Manager: Got Current User";
        core_->UserManager().GetCurrentUser(currentUser_);
        QString username(currentUser_->GetUsername());
        qDebug() << "Discord Manager: Emitting username" << username;
        emit onUserConnected(username);
        ready_ = true;
        auto registerResult = core_->ActivityManager().RegisterCommand("yate://run");
        if (registerResult != discord::Result::Ok) {
            qWarning() << "Discord Manager: Failed to register result " << int(registerResult);
        }
    }

    core_->UserManager().OnCurrentUserUpdate.Connect([&]() {
        qDebug() << "Discord Manager: Current User Update";
        auto userResult =core_->UserManager().GetCurrentUser(currentUser_);
        if (userResult == discord::Result::Ok) {
            QString username(currentUser_->GetUsername());
            qDebug() << "Discord Manager: Emitting username" << username;
            emit onUserConnected(username);
            ready_ = true;
            auto registerResult = core_->ActivityManager().RegisterCommand("yate://run");
            if (registerResult != discord::Result::Ok) {
                qWarning() << "Discord Manager: Failed to register result " << int(registerResult);
            }
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
                emit onLobbyIdChange(QString::fromUtf8(lobbySecret_));

            } else {
                qCritical() << "Discord Manager: Failed to create lobby" << int(result);
            }
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
        const char *lobbySecretArry = lobbySecretBA.data();
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
        core_->LobbyManager().ConnectLobbyWithActivitySecret(lobbySecretArry, [&](discord::Result result, const discord::Lobby &lobby) {
            if (result == discord::Result::Ok) {
                qDebug() << "Discord Manager: Connected to lobby " << lobby.GetId() << lobby.GetOwnerId();
                peerUserId_ = lobby.GetOwnerId();
                peerLobbyId_ = lobby.GetId();
                qDebug() << "Discord Manager: Emitting connection succeeded";
                emit connectionSucceeded();
            } else {
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
