#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H


#ifdef DISCORD_ENABLED
#include <QString>
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


    bool running() const;


    bool isHost() const;
    void setIsHost(bool newIsHost);

    const QString &getLobbyId() const;
    void setLobbyId(const QString &newLobbyId);

    const QString &getPeerLobbyId() const;
    void setPeerLobbyId(const QString &newPeerLobbyId);

public slots:
    void start();
    void stop();
    void update();
    void updateActivity();
    void clearActivity();
    void setActivityState(const QString &newActivityState);
    void setActivityDetails(const QString &newActivityDetails);
    void setSquadString(QString msg);


signals:
    void ready();
    void failed(QString err);
    void onLobbyIdChange(QString id);
    void onPeerIdChange(QString id);
    void onUserConnected(QString name);
    void connectionSucceeded();
    void connectionFailed();
    void onLobbyDisconnect();
    void onInviteAccepted(QString secret);

private:
    void setup(bool emitErrors = true);
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
    int currentPartySize_;
    int activityPartySize_;
    QString activityLobbyId_;
    QString lobbyId_;
    QString lobbyUsername_;
    QString peerLobbyId_;
    char appCommand[2048];
    bool isHost_;
    bool activityIsHost_;
    bool commandRegistered_;
};

}
#endif

#endif // DISCORDMANAGER_H
