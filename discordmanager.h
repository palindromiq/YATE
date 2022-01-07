#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H
#include "globals.h"

#ifdef DISCORD_ENABLED
#include <QObject>
#include <QSet>
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

    const QString &host() const;

    const QSet<QString> &squad() const;

    bool connectTo(QString lobbySecret);


    bool running() const;

public slots:
    void start();
    void stop();
    void update();
    void updateActivity();
    void clearActivity();
    void setActivityState(const QString &newActivityState);
    void setActivityDetails(const QString &newActivityDetails);
    void setSquad(const QSet<QString> &newSquad);
    void setHost(const QString &newHost);
    void sendMessageOnChannel1(QString msg);
    void sendMessageOnChannel2(QString msg);
    void disconnectFromLobby();
    void checkMessageBuffers();


signals:
    void ready();
    void failed(QString err);
    void onLobbyIdChange(QString id);
    void onPeerIdChange(QString id);
    void onUserConnected(QString name);
    void onMessagrFromChannel1(QString msg);
    void onMessagrFromChannel2(QString msg);
    void connectionSucceeded();
    void connectionFailed();
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
    QString host_;
    QSet<QString> squad_;
    QSettings *settings_;
    QString ch1Buffer;
    QString ch2Buffer;
    QMutex bufferMutex_;
    bool ready_;
    bool failed_;
    bool running_;
    bool activityInit_;
    long long lobbyId_;
    long long peerLobbyId_;
    long long peerUserId_;
};

}
#endif

#endif // DISCORDMANAGER_H
