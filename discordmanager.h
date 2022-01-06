#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H
#include "globals.h"

#ifdef DISCORD_ENABLED
#include <QObject>
#include <QSet>

namespace discord {
    class Core;
    class User;
}

class QTimer;

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


signals:
    void ready();
    void failed(QString err);
private:
    void setup(bool emitErrors = true);
    discord::Core* core_;
    discord::User* currentUser_;
    QTimer *updateTimer_;
    QString currentActivityDetails_;
    QString currentActivityState_;
    QString currentActivityImageText_;
    QString activityDetails_;
    QString activityState_;
    QString activityImageText_;
    QString host_;
    QSet<QString> squad_;
    bool ready_;
    bool failed_;
    bool running_;
};

}
#endif

#endif // DISCORDMANAGER_H
