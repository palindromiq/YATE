#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMqttTopicName>

class QMqttClient;
class QSettings;
class QTimer;

namespace Yate {

class MqttManager : public QObject
{
    Q_OBJECT
public:
    explicit MqttManager(QObject *parent = nullptr, QString key = "");

    bool connectTo(QString lobby, QString &errMsg);
    QString topic();
    QString topicUrl();

    const QString &getUserId() const;

public slots:
    void start();
    void sendMessageOnChannel1(QString msg);
    void sendMessageOnChannel2(QString msg);
    void sendMessageOnChannel3(QString msg);
    void disconnectFromLobby();
    void reconnect();

private slots:
     void messageReceived(const QByteArray &message, const QMqttTopicName &topic);
     void checkMessageBuffers();
     void updateLogStateChange();
     void brokerDisconnected();
     void publishMessage(QString msg);

signals:
    void onMessageFromChannel1(QString msg);
    void onMessageFromChannel2(QString msg);
    void onMessageFromChannel3(QString msg);
    void onLobbyIdChange(QString id);
    void connectionFailed(QString msg);
    void connected();
    void connecting();
    void disconnected();

private:
    void sendMessageOnChannel(QString msg, int ch);
    bool running_;
    bool initialized_;
    QMqttClient *client_;
    QTimer *messageBufferTimer_;
    QSettings *settings_;
    QString ch1Buffer_;
    QString ch2Buffer_;
    QString ch3Buffer_;
    QMutex bufferMutex_;
    QString username_;
    QString password_;
    QMqttTopicName peerTopic_;
    QString peerId_;
    QString userId_;
    QString userPassword_;
    int clientState_;
    bool isSecure_;

    QString stateToString_(int state);

};

}

#endif // MQTTMANAGER_H
