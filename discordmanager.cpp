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
    connect(updateTimer_, &QTimer::timeout, this, &DiscordManager::update);
    connect(messageBufferTimer_, &QTimer::timeout, this, &DiscordManager::checkMessageBuffers);
    setup();
}

DiscordManager::~DiscordManager()
{
    if(lobbyId_ != -1) {
        if (core_) {
            core_->LobbyManager().DeleteLobby(lobbyId_, [&](discord::Result) {});
        }
    }
    delete core_;
    delete currentUser_;
}

void DiscordManager::start()
{
    if (running()) {
        return;
    }
    updateTimer_->start(DISCORD_UPDATE_TIMER);
    messageBufferTimer_->start(DISCORD_MESSAGE_BUFFER_TIMER);
    running_ = true;
}

void DiscordManager::stop()
{
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


    DiscordNetworkPeerId peerId;
    core_->NetworkManager().GetPeerId(&peerId);
    emit onPeerIdChange(QString::number(peerId));

    core_->UserManager().OnCurrentUserUpdate.Connect([&]() {
        core_->UserManager().GetCurrentUser(currentUser_);
        QString username(currentUser_->GetUsername());
        emit onUserConnected(username);
        ready_ = true;

    });
    core_->LobbyManager().OnLobbyMessage.Connect([&](discord::LobbyId lobbyId, discord::UserId userId,  std::uint8_t* data, std::uint32_t dataLen){
        if (lobbyId == peerLobbyId_ && userId == peerUserId_) {
            QJsonDocument jsonMsg = QJsonDocument::fromJson(QString::fromUtf8((char*)data, dataLen).toUtf8());
            QJsonObject obj = jsonMsg.object();
            if (obj.contains("message")) {
                QString message = obj.value("message").toString();
                if (obj.contains("channel")) {
                    QString ch = obj.value("channel").toString();
                    if (ch == "1") {
                        emit onMessagrFromChannel1(message);
                    } else if (ch == "2") {
                        emit onMessagrFromChannel2(message);
                    }
                }
            }
        }
    });
    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {

        discord::LobbyTransaction txn;
        core_->LobbyManager().GetLobbyCreateTransaction(&txn);
        txn.SetType(discord::LobbyType::Public);

        core_->LobbyManager().CreateLobby(txn, [&](discord::Result result, const discord::Lobby &lobby) {
            if (result == discord::Result::Ok) {
                char secret[512];
                core_->LobbyManager().GetLobbyActivitySecret(lobby.GetId(), secret);
                lobbyId_ = lobby.GetId();
                emit onLobbyIdChange(QString::fromUtf8(secret));
            } else {
                qDebug() << "Failed";
            }
        });


    }


}

void DiscordManager::sendMessageToLobby(QString msg)
{
    if(lobbyId_ != -1) {
        std::uint8_t *msgPayload = (std::uint8_t*) msg.toUtf8().data();
        core_->LobbyManager().SendLobbyMessage(lobbyId_, msgPayload, msg.size(), [&](discord::Result result) {
            if (result != discord::Result::Ok) {
                qDebug() << "Failed to send message" << int(result);
            }
        });
    }
}



bool DiscordManager::running() const
{
    return running_;
}

const QSet<QString> &DiscordManager::squad() const
{
    return squad_;
}

bool DiscordManager::connectTo(QString lobbySecret)
{

    if (settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings_->value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {
        QByteArray lobbySecretBA = lobbySecret.toUtf8();
        const char *lobbySecretArry = lobbySecretBA.data();
        auto split = lobbySecret.split(":");
        if (split.size() != 2) {
            return false;
        }
        if (split[0] == QString::number(lobbyId_)) {
            return false;
        }
        core_->LobbyManager().ConnectLobbyWithActivitySecret(lobbySecretArry, [&](discord::Result result, const discord::Lobby &lobby) {
            if (result == discord::Result::Ok) {
                peerUserId_ = lobby.GetOwnerId();
                peerLobbyId_ = lobby.GetId();
                emit connectionSucceeded();
            } else {
                qDebug() << "Failed";
                emit connectionFailed();
            }

        });

    }

    return true;

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

void DiscordManager::sendMessageOnChannel1(QString msg) {
    if(lobbyId_ != -1) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "1.0");
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", "1");
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        ch1Buffer = msgJson;
    }

}

void DiscordManager::sendMessageOnChannel2(QString msg) {
    if(lobbyId_ != -1) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "1.0");
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", "2");
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        ch2Buffer = msgJson;
    }
}

void DiscordManager::disconnectFromLobby() {
    if (peerLobbyId_ != -1) {
        core_->LobbyManager().DisconnectLobby(peerLobbyId_, [&](discord::Result result) {
            if (result != discord::Result::Ok) {
                qDebug() << "Disconnect failed " << int(result);
            }
        });
        lobbyId_ = -1;
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
