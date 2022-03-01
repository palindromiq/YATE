#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H
#include "globals.h"

#ifdef DISCORD_ENABLED
#include <QObject>
#include <QMutex>


namespace discord {
    class Core;
    class User;
}

class QTimer;
class QSettings;

namespace Yate {

class DiscordManager : public QObject
{
    Q_OBJECT
public:
    explicit DiscordManager(QObject *parent = nullptr);
    ~DiscordManager();
    const QString &activityDetails() const;

    const QString &activityState() const;

    bool connectTo(QString lobbySecret);


    bool running() const;

    long long getLobbyId() const;

    bool isHost() const;
    void setIsHost(bool newIsHost);

public slots:
    void start();
    void stop();
    void update();
    void updateActivity();
    void clearActivity();
    void setActivityState(const QString &newActivityState);
    void setActivityDetails(const QString &newActivityDetails);
    void sendMessageOnChannel1(QString msg);
    void sendMessageOnChannel2(QString msg);
    void sendMessageOnChannel3(QString msg);
    void disconnectFromLobby();
    void checkMessageBuffers();
    void setSquadString(QString msg);


signals:
    void ready();
    void failed(QString err);
    void onLobbyIdChange(QString id);
    void onPeerIdChange(QString id);
    void onUserConnected(QString name);
    void onMessageFromChannel1(QString msg);
    void onMessageFromChannel2(QString msg);
    void onMessageFromChannel3(QString msg);
    void connectionSucceeded();
    void connectionFailed();
    void onLobbyDisconnect();
    void onInviteAccepted(QString secret);

private:
    void setup(bool emitErrors = true);
    void sendMessageToLobby(QString msg);
    discord::Core* core_;
    discord::User* currentUser_;
    QTimer *updateTimer_;
    QTimer *messageBufferTimer_;
    QString currentActivityDetails_;
    QString currentActivityState_;
    QString currentActivityImageText_;
    QString activityDetails_;
    QString activityState_;
    QString activityImageText_;
    QSettings *settings_;
    QString ch1Buffer;
    QString ch2Buffer;
    QString ch3Buffer;
    QMutex bufferMutex_;
    QAtomicInt ready_;
    QAtomicInt failed_;
    QAtomicInt running_;
    QAtomicInt activityInit_;
    long long lobbyId_;
    long long peerLobbyId_;
    long long peerUserId_;
    int currentPartySize_;
    int activityPartySize_;
    long long activityLobbyId_;
    char lobbySecret_[512];
    char lobbyIdStr_[512];
    char peerLobbyIdStr_[512];
    char peerLobbySecret_[512];
    char appCommand[2048];
    bool isHost_;
    bool activityIsHost_;
    bool commandRegistered_;
};

}
#endif

#endif // DISCORDMANAGER_H
