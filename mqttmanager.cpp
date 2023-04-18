#include "mqttmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QMqttClient>
#include <QSslConfiguration>
#include <QUuid>
#include <QSettings>

#include "updater.h"
#include "globals.h"

namespace Yate {

MqttManager::MqttManager(QObject *parent, QString key)
    : QObject{parent}, running_(false), messageBufferTimer_(new QTimer(this)), settings_(new QSettings), clientState_(QMqttClient::Disconnected)
{

    connect(messageBufferTimer_, &QTimer::timeout, this, &MqttManager::checkMessageBuffers);
    auto keyparts = key.trimmed().split(":");
    if (keyparts.size() != 2) {
        qCritical() << "MQTT Manager: received invalid key.";
        initialized_ = false;
        return;
    }
    bool isSecure_ =  QSslSocket::supportsSsl() && !QSslConfiguration::defaultConfiguration().isNull();
    if (!isSecure_) {
        qCritical() << "MQTT Manager: Secure connection is required.";
        initialized_ = false;
        return;
    }

    client_ = new QMqttClient(this);
    client_->setHostname(MQTT_HOST);
    client_->setPort(MQTT_PORT_SECURE);
    client_->setProtocolVersion(QMqttClient::MQTT_3_1_1);
    client_->setKeepAlive(MQTT_KEEPALIVE);
    username_ = keyparts[0];
    password_ = keyparts[1];
    userId_ = YATE_2_0_PREFIX + QString(QUuid::createUuid().toRfc4122().toHex()).mid(12);
    userPassword_ = QString(QUuid::createUuid().toRfc4122().toHex()).mid(0, 20);
    settings_->setValue(SETTINGS_KEY_MQTT_USER_ID, userId_);
    settings_->setValue(SETTINGS_KEY_MQTT_USER_PASS, userPassword_);
    client_->setUsername(username_);
    client_->setPassword(password_);

    qDebug() << "Connecting" << MQTT_HOST << " " << MQTT_PORT_SECURE << " secure?" << isSecure_;
    qDebug() << "User ID" << userId_;
    qDebug() << "User Password" << userPassword_;

    connect(client_, &QMqttClient::stateChanged, this, &MqttManager::updateLogStateChange);
    connect(client_, &QMqttClient::disconnected, this, &MqttManager::brokerDisconnected);



    connect(client_, &QMqttClient::messageReceived, this, &MqttManager::messageReceived);
//    connect(client_, &QMqttClient::pingResponseReceived, this, []() {
//           qDebug() << "ping";
//    });

    client_->connectToHostEncrypted(QSslConfiguration::defaultConfiguration());
    initialized_ = true;
}

bool MqttManager::connectTo(QString lobby, QString &errMsg)
{
    auto lobbyParts = lobby.trimmed().split(":");
    if (lobbyParts.size() != 2 || !lobby.startsWith(YATE_2_0_PREFIX)) {
        errMsg = "Invalid lobby ID";
        emit connectionFailed(errMsg);
        return false;
    }
    lobby = lobbyParts[0] + "/" + lobbyParts[1];
    if (client_->state() == QMqttClient::Connecting) {
        errMsg = "Still connecting to the server, try again in few seconds.";
        emit connectionFailed(errMsg);
        return false;
    } else if (client_->state() == QMqttClient::Disconnected) {
        errMsg = "Failed to connect to the server";
        emit connectionFailed(errMsg);
        return false;
    } else {
        if (peerTopic_.isValid()) {
            client_->unsubscribe(QMqttTopicFilter(peerTopic_.name()));
        }
        peerTopic_.setName(lobby);
        auto subscription = client_->subscribe(QMqttTopicFilter(peerTopic_.name()));
        if (!subscription) {
           errMsg = "Failed to connect to the lobby";
           emit connectionFailed(errMsg);
           return false;
        }
    }
    return true;
}

QString MqttManager::topic() {
    return userId_ + "/" + userPassword_;
}
QString MqttManager::topicUrl() {
    return userId_ + ":" + userPassword_;
}

void MqttManager::start()
{
    messageBufferTimer_->start(MQTT_MESSAGE_BUFFER_TIMER);
    running_ = true;
}

void MqttManager::sendMessageOnChannel1(QString msg)
{
   sendMessageOnChannel(msg, 1);
}

void MqttManager::sendMessageOnChannel2(QString msg)
{
    sendMessageOnChannel(msg, 2);
}

void MqttManager::sendMessageOnChannel3(QString msg)
{
    sendMessageOnChannel(msg, 3);
}

void MqttManager::disconnectFromLobby()
{
    if (peerTopic_.isValid()) {
        client_->unsubscribe(QMqttTopicFilter(peerTopic_.name()));
        peerTopic_.setName("");
    }
}

void MqttManager::reconnect() {
    client_->connectToHostEncrypted(QSslConfiguration::defaultConfiguration());
}

void MqttManager::messageReceived(const QByteArray &message, const QMqttTopicName &)
{
    if (peerTopic_.isValid()) {
        qDebug() << "MQTT Manager: Message accepted";
        QJsonDocument jsonMsg = QJsonDocument::fromJson(message);
        QJsonObject obj = jsonMsg.object();
        if (!obj.contains("from") || obj.value("from").toString() != peerTopic_.name()) {
            qWarning() << "Received message from invalid topic "
                       << (obj.contains("from")? obj.value("from").toString(): "<no from>")
                       << ", expected " << peerTopic_.name();
            return;
        }
        if (obj.contains("message") && obj.contains("type") && obj.value("type").toString() == "default") {
            QString message = obj.value("message").toString();
            if (obj.contains("channel")) {
                QString ch = obj.value("channel").toString();
                if (ch == "1") {
                    qDebug() << "MQTT Manager: Message received on channel 1";
                    emit onMessageFromChannel1(message);
                } else if (ch == "2") {
                    qDebug() << "MQTT Manager: Message received on channel 2";
                    emit onMessageFromChannel2(message);
                } else if (ch == "3") {
                    qDebug() << "MQTT Manager: Message received on channel 3";
                    emit onMessageFromChannel3(message);
                }
            }
        }
    } else {
        qDebug () << "MQTT Manager: No topic";
    }
}

void MqttManager::checkMessageBuffers()
{
    QMutexLocker lock(&bufferMutex_);
    if (ch1Buffer_.size()) {
        QString payload = ch1Buffer_;
        ch1Buffer_ = "";
        publishMessage(payload);
    }
    if (ch2Buffer_.size()) {
        QString payload = ch2Buffer_;
        ch2Buffer_ = "";
        publishMessage(payload);
    }
    if (ch3Buffer_.size()) {
        QString payload = ch3Buffer_;
        ch3Buffer_ = "";
        publishMessage(payload);
    }
}

void MqttManager::updateLogStateChange() {
    clientState_ = client_->state();
    qDebug() << "MQTT Manager: state changed " << clientState_ << "(" << stateToString_(clientState_) << ")";
    switch(clientState_) {
    case QMqttClient::Connected:
        qDebug () << "MQTT Manager: Connected!";
        emit connected();
        emit onLobbyIdChange(topicUrl());
        break;
    case QMqttClient::Connecting:
        qDebug () << "MQTT Manager: Connecting!";
        emit connecting();
        break;
    case QMqttClient::Disconnected:
        qDebug () << "MQTT Manager: Disconnected!";
        emit disconnected();
        break;
    }

}

void MqttManager::brokerDisconnected()
{
    qDebug() << "MQTT Manager: disconnected";
}

void MqttManager::publishMessage(QString msg)
{
    if (clientState_ == QMqttClient::Connected) {
        qDebug() << "MQTT Manager: publishing to" << topic();
        auto publishResult = client_->publish(topic(), msg.toUtf8());
        qDebug() << "MQTT Manager: publish result:" << publishResult;
    } else {
        qWarning() << "MQTT Manager: not connected.";
    }
}

void MqttManager::sendMessageOnChannel(QString msg, int ch)
{
    qDebug() << "MQTT Manager: sendMessageOnChannel" << ch;
    if(clientState_ == QMqttClient::Connected) {
        QJsonObject  json;
        json.insert("message", msg);
        json.insert("version", "2.0");
        json.insert("protocol", "mqtt");
        json.insert("type", "default");
        json.insert("from", topic());
        json.insert("yate_ver", Updater::getInstance(0)->getVersion());
        json.insert("channel", QString::number(ch));
        QString msgJson(QJsonDocument(json).toJson(QJsonDocument::Compact));
        QMutexLocker lock(&bufferMutex_);
        if (ch == 1) {
            ch1Buffer_ = msgJson;
        } else if (ch == 2) {
            ch2Buffer_ = msgJson;
        } else if (ch == 3) {
            ch3Buffer_ = msgJson;
        } else {
             qWarning() << "MQTT Manager: channel" << ch << " is not supported";
        }

    } else {
        qWarning() << "MQTT Manager: not connected.";
    }
}

const QString &MqttManager::getUserId() const
{
    return userId_;
}

QString MqttManager::stateToString_(int state)
{
    if (state == QMqttClient::Disconnected) {
        return "Disconnected";
    } else if (state == QMqttClient::Connected) {
        return "Connected";
    } else if (state == QMqttClient::Connecting) {
        return "Connecting";
    }
    return "";
}

}
