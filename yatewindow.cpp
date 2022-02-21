#include "yatewindow.h"
#include "ui_yatewindow.h"
#include <QStandardPaths>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QAction>
#include <QMenu>
#include <QThread>
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include <QClipboard>
#include <QInputDialog>



#include "globals.h"
#include "settingsdialog.h"
#include "analysiswindow.h"
#include "livefeedbackoverlay.h"
#include "eeparser.h"
#include "huntinfogenerator.h"
#include "updater.h"

#ifdef DISCORD_ENABLED
#include "discordmanager.h"
#endif

namespace Yate {




YATEWindow::YATEWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::YATEWindow),
      settingsDialog_(new SettingsDialog(this)),
      feedbackOverlay_(nullptr),
      analysisWindow_(new AnalysisWindow(this)),
      parser_(nullptr),
      huntInfoGenerator_(nullptr),
      isLogManuallySet_(false),
#ifdef DISCORD_ENABLED
      discord_(nullptr),
#endif
      isLiveFeedbackRunning_(false)


{
    qDebug() << "Initializing main window.";
    ui->setupUi(this);
    setFixedSize(size());
    QSettings settings;
    if (settings.value(SETTINGS_KEY_EE_LOG).isNull()) {
        QString appDataPath;
        QStringList appDataLocations = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
        if (!appDataLocations.length()) {
            appDataPath = QDir::homePath() + QDir::separator() + "AppData" + QDir::separator() + "Local";
        } else {
            appDataPath = appDataLocations.first();
            appDataPath = QFileInfo(appDataPath).dir().absolutePath(); // Up one directory
            appDataPath = QFileInfo(appDataPath).dir().absolutePath(); // Up one directory
        }
        eeLogFile_.setFileName(QDir(appDataPath).filePath(PATH_EE_LOG_RELATIVE));
    } else {
        eeLogFile_.setFileName(settings.value(SETTINGS_KEY_EE_LOG).toString());
    }

    QString eePath(QFileInfo(eeLogFile_).filePath());
    Updater::getInstance(this);


    ui->lblLogFilePath->setText(eePath);

    settingsDialog_->setEEFilePath(eePath);
    settings.setValue(SETTINGS_KEY_EE_LOG, eePath);

    if (!eeLogFile_.exists()) {
        statusBar()->showMessage(tr("Log file not found!"), 3000);
    }


    createTrayIcon();
    connect(this, &YATEWindow::exitFeebackOverlay, this, &YATEWindow::stopFeedback);

    setAcceptDrops(true);
    auto updater = Updater::getInstance(this);
    connect(this, &YATEWindow::checkForUpdate, updater, &Updater::checkForUpdate);
    connect(settingsDialog_, &SettingsDialog::checkForUpdate, updater, &Updater::checkForUpdate);
    connect(updater, &Updater::updateStatusUpdate, settingsDialog_, &SettingsDialog::onUpdateStatusUpdate);
    connect(updater, &Updater::updateAvailable, settingsDialog_, &SettingsDialog::onUpdateAvailable);
    connect(updater, &Updater::errorOccurred, settingsDialog_, &SettingsDialog::onUpdaterError);
    connect(updater, &Updater::onBusyUpdate, settingsDialog_, &SettingsDialog::lockUpdateBtn);
    if (settings.value(SETTINGS_KEY_UPDATE_ON_STARTUP, true).toBool()) {
        emit checkForUpdate();
    }
#ifdef DISCORD_ENABLED
    if (settings.value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool()) {
        initDiscord();
    } else {
        discordThread_ = nullptr;
        discord_ = nullptr;
    }
#else
    ui->btnLiveFeedbackVS->setVisible(false);
    ui->btnLiveFeedback->setText(tr("Live Feedback"));
#endif

    qDebug() << "Main window initialized.";

}

void Yate::YATEWindow::initDiscord()
{
#ifdef DISCORD_ENABLED
    qDebug() << "Initializing Discord.";
    discordThread_ = new QThread;
    discord_ = new DiscordManager;
    discord_->moveToThread(discordThread_);
    connect(discordThread_, &QThread::started, discord_, &DiscordManager::start);
    connect(discordThread_, &QThread::finished, discordThread_, &QThread::deleteLater);
    connect(discord_, &DiscordManager::onLobbyIdChange, this, &YATEWindow::onLobbyIdChange, Qt::UniqueConnection);
    connect(this, &YATEWindow::discordStart, discord_, &DiscordManager::start, Qt::UniqueConnection);
    connect(this, &YATEWindow::discordStop, discord_, &DiscordManager::stop, Qt::UniqueConnection);
    connect(this, &YATEWindow::discordClearActivity, discord_, &DiscordManager::clearActivity, Qt::UniqueConnection);
    connect(discord_, &DiscordManager::onUserConnected, this, &YATEWindow::onUserConnected, Qt::UniqueConnection);
    connect(this, &YATEWindow::disconnectDiscordLobby, discord_, &DiscordManager::disconnectFromLobby, Qt::UniqueConnection);
    discordThread_->start();
    qDebug() << "Discord thread started.";
#endif
}


void YATEWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        qDebug() << "Accepted log file via drop.";
        e->acceptProposedAction();
    }
}

void YATEWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        qDebug() << "Setting log file via drop.";
        QString fileName = url.toLocalFile();
        ui->lblLogFilePath->setText(fileName);
        eeLogFile_.setFileName(fileName);
        isLogManuallySet_ = true;
        break;
    }
}

YATEWindow::~YATEWindow()
{
    qDebug() << "Closing YATE";
#ifdef DISCORD_ENABLED
    if(discord_) {
        qDebug() << "Deleting Discord Manager.";
        emit discordStop();
        qDebug() << "Waiting for Discord Managers timers.";
        while(discord_->running()) {

        }
        qDebug() << "Finished waiting for Discord Managers timers.";
        delete discord_;
        qDebug() << "Deleted Discord Manager.";
    }
#endif
    delete ui;
}


void YATEWindow::on_btnSettings_clicked()
{
    qDebug() << "Starting settings dialog.";
    settingsDialog_->reloadSettings();
    settingsDialog_->exec();
}


void YATEWindow::on_btnEditLogPath_clicked()
{
    QString newPath = QFileDialog::getOpenFileName(this, tr("EE.log file"), QFileInfo(eeLogFile_.fileName()).dir().absolutePath(), tr("Log File (*.log)"));
    if(newPath.length()) {
        qDebug() << "Log file path edited.";
        ui->lblLogFilePath->setText(newPath);
        eeLogFile_.setFileName(newPath);
        isLogManuallySet_ = true;
    }
}


void YATEWindow::on_btnShrineWater_clicked()
{
    disableUI();
    qDebug() << "Starting offline analysis.";
#ifdef THREADED_PARSING
    parser_ = new EEParser(eeLogFile_.fileName(), false);
    huntInfoGenerator_ = new HuntInfoGenerator;
    QThread *parserThread = new QThread;
    parser_->moveToThread(parserThread);

    connect(parser_, &EEParser::parsingStarted, this, &YATEWindow::onParserStarted);
    connect(parser_, &EEParser::parsingFinished, this, &YATEWindow::onParserFinished);
    connect(parser_, &EEParser::parsingError, this, &YATEWindow::onParserError);
    connect(parser_, &EEParser::logEvent, huntInfoGenerator_, &HuntInfoGenerator::onLogEvent);

    connect( parserThread, &QThread::started, parser_, &EEParser::startOffline);
    connect( parser_, &EEParser::parsingFinished, parserThread, &QThread::quit);
    connect( parser_, &EEParser::parsingFinished, parser_, &EEParser::deleteLater);
    connect( parser_, &EEParser::parsingFinished, huntInfoGenerator_, &HuntInfoGenerator::deleteLater);
    connect( parserThread, &QThread::finished, parserThread, &QThread::deleteLater);

    parserThread->start();
    qDebug() << "Offline parsing thread started.";
#else
    if (parser_ == nullptr) {
        parser_ = new EEParser(eeLogFile_.fileName(), false, this);
        huntInfoGenerator_ = new HuntInfoGenerator(this);
    } else {
        parser_->setFilename(eeLogFile_.fileName());
        parser_->reset();
        huntInfoGenerator_->resetHuntInfo();
        disconnect(parser_, &EEParser::logEvent, huntInfoGenerator_, &HuntInfoGenerator::onLogEvent);
    }
    connect(parser_, &EEParser::parsingStarted, this, &YATEWindow::onParserStarted);
    connect(parser_, &EEParser::parsingFinished, this, &YATEWindow::onParserFinished);
    connect(parser_, &EEParser::parsingError, this, &YATEWindow::onParserError);
    connect(parser_, &EEParser::logEvent, huntInfoGenerator_, &HuntInfoGenerator::onLogEvent);
    parser_->start();
#endif
}


void Yate::YATEWindow::showLiveFeedback(bool lock)
{
    qDebug() << "Starting host live feedback." << (lock? " (Locked)": " (Unlocked)");
    feedbackOverlay_ = new LiveFeedbackOverlay(nullptr, lock);
    trayStopFeedback_->setVisible(true);
    parser_ = new EEParser(eeLogFile_.fileName(), false);
    huntInfoGenerator_ = new HuntInfoGenerator;
    QThread *parserThread = new QThread;
    parser_->moveToThread(parserThread);

    connect(parser_, &EEParser::parsingStarted, this, &YATEWindow::onParserStarted);
    connect(parser_, &EEParser::parsingError, this, &YATEWindow::onParserError);
    connect(parser_, &EEParser::parsingError, feedbackOverlay_, &LiveFeedbackOverlay::onUpdateMessage);
    connect(parser_, &EEParser::logEvent, huntInfoGenerator_, &HuntInfoGenerator::onLogEvent);
    connect(parser_, &EEParser::parsingReset, huntInfoGenerator_, &HuntInfoGenerator::resetHuntInfo);



    connect(parserThread, &QThread::started, parser_, &EEParser::startLive);
    connect(parser_, &EEParser::parsingFinished, parserThread, &QThread::quit);
    connect(parser_, &EEParser::parsingFinished, parser_, &EEParser::deleteLater);
    connect(parser_, &EEParser::parsingFinished, huntInfoGenerator_, &HuntInfoGenerator::deleteLater);
    connect(huntInfoGenerator_, &HuntInfoGenerator::onHuntStateChanged, feedbackOverlay_, &LiveFeedbackOverlay::onUpdateMessage);
    connect(huntInfoGenerator_, &HuntInfoGenerator::onLimbsChanged, feedbackOverlay_, &LiveFeedbackOverlay::onUpdateLimbs);
    connect(parserThread, &QThread::finished, parserThread, &QThread::deleteLater);
    connect(this, &YATEWindow::exitFeebackOverlay, parser_, &EEParser::stopParsing);
    connect(feedbackOverlay_, &LiveFeedbackOverlay::onDoubleClicked, parser_, &EEParser::stopParsing);
    connect(feedbackOverlay_, &LiveFeedbackOverlay::onDoubleClicked, this, &YATEWindow::stopFeedback);
    connect(feedbackOverlay_, &LiveFeedbackOverlay::onLockWindow, this, &YATEWindow::lockFeedbackWindow);

#ifdef DISCORD_ENABLED
    QSettings settings;
    if (settings.value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool()) {
        if (settings.value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {
            connect(huntInfoGenerator_, &HuntInfoGenerator::onHuntStateChanged, discord_, &DiscordManager::sendMessageOnChannel1);
            connect(huntInfoGenerator_, &HuntInfoGenerator::onLimbsChanged, discord_, &DiscordManager::sendMessageOnChannel2);
            connect(huntInfoGenerator_, &HuntInfoGenerator::onHostOrSquadChanged, discord_, &DiscordManager::sendMessageOnChannel3);
        }
        if (settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
            connect(huntInfoGenerator_, &HuntInfoGenerator::onHuntStateChanged, discord_, &DiscordManager::setActivityDetails);
            connect(huntInfoGenerator_, &HuntInfoGenerator::onLimbsChanged, discord_, &DiscordManager::setActivityState);
            connect(huntInfoGenerator_, &HuntInfoGenerator::onHostOrSquadChanged, discord_, &DiscordManager::setSquadString);
            connect(this, &YATEWindow::feedbackWindowClosed, discord_, &DiscordManager::clearActivity);
        }

    }
#endif

    parserThread->start();
    isLiveFeedbackRunning_ = true;

    feedbackOverlay_->onUpdateMessage(LIVE_FEEDBACK_DEFAULT_MSG);

    feedbackOverlay_->showNormal();
    trayIcon_->show();
    showMinimized();
    qDebug() << "Host live feedback started" << (lock? " (Locked)": " (Unlocked)");;
}

void YATEWindow::on_btnLiveFeedback_clicked()
{
    showLiveFeedback(false);
}

void YATEWindow::showWindow()
{
    show();
}
void YATEWindow::changeEvent(QEvent* e)
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QWindowStateChangeEvent* event = static_cast<QWindowStateChangeEvent*>(e);

        if( event->oldState() & Qt::WindowMinimized )
        {
            qDebug() << "Window restore, closing feedback window if open";
            if(isLiveFeedbackRunning_) {
                stopFeedback();
            }
        }
    }
}

void YATEWindow::stopFeedback()
{
    qDebug() << "Stopping live feedback.";
    if (isLiveFeedbackRunning_.fetchAndStoreAcquire(false)) {
        trayStopFeedback_->setVisible(false);
        feedbackOverlay_->close();
        trayIcon_->hide();
        showNormal();
        qDebug() << "Stopped live feedback.";
        emit feedbackWindowClosed();
        emit disconnectDiscordLobby();
        qDebug() << "Emitted closing signals.";
    } else {
        qDebug () << "Live feedback already stopped.";
    }

}

void YATEWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        qDebug() << "Activated window through tray icon.";
        show();
        emit exitFeebackOverlay();
        break;

    default:
        ;
    }

}

void YATEWindow::exitApp()
{

}

void YATEWindow::disableUI()
{
    ui->btnEditLogPath->setEnabled(false);
    ui->btnLiveFeedback->setEnabled(false);
    ui->btnLiveFeedbackVS->setEnabled(false);
    ui->btnCopyLobbyId->setEnabled(false);
    ui->btnShrineWater->setEnabled(false);
    ui->btnSettings->setEnabled(false);
}

void YATEWindow::enableUI()
{
    ui->btnEditLogPath->setEnabled(true);
    ui->btnLiveFeedback->setEnabled(true);
    ui->btnLiveFeedbackVS->setEnabled(true);
    ui->btnCopyLobbyId->setEnabled(true);
    ui->btnShrineWater->setEnabled(true);
    ui->btnSettings->setEnabled(true);
}

void YATEWindow::onParserStarted()
{

}

void YATEWindow::onParserFinished()
{
    qDebug() << "Parser finished.";
    enableUI();
    // Set analysisWindow_ model
    analysisWindow_->setHunt(huntInfoGenerator_->huntInfo());
    analysisWindow_->show();
}

void YATEWindow::onParserError(QString err)
{
    qCritical() << "Parser error: " << err;
    enableUI();
}

void YATEWindow::lockFeedbackWindow()
{
    qDebug() << "Locking host live feedback window";
    feedbackOverlay_->close();
    showLiveFeedback(true);
    qDebug() << "Locked host live feedback window";
}
void YATEWindow::lockClientFeedbackWindow()
{
    qDebug() << "Locking VS live feedback window";
    feedbackOverlay_->close();
    showClientLiveFeedback(true);
    qDebug() << "Locked VS live feedback window";
}

void YATEWindow::setLogFilePath(QString path)
{
    qDebug() << "Changing log file path";
    ui->lblLogFilePath->setText(path);
    eeLogFile_.setFileName(path);
    isLogManuallySet_ = true;
}

void YATEWindow::onUpdaterBusy(bool busy) {
    setEnabled(!busy);
}

void YATEWindow::refreshDiscordSettings()
{
#ifdef DISCORD_ENABLED
    qDebug() << "Refreshing Discord settings";
    QSettings settings;
    if (settings.value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool()) {
        if (!discord_) {
            initDiscord();
        } else {
            emit discordStart();
        }
        if (!settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
            emit discordClearActivity();
        }

    } else {
        emit discordStop();
    }
#endif
}

void YATEWindow::onLobbyIdChange(QString id) {
    qDebug() << "Lobby ID received: " << id;
    ui->lblLobbyId->setText(id);
}

void YATEWindow::onUserConnected(QString name)
{
    qDebug() << "Received Discord user" << name;
    QString title = "YATE " + Updater::getInstance()->getVersion();
    if(name.size()) {
        title = title + " (Logged in to Discord as @" + name + ")";
        statusBar()->showMessage("Logged in to Discord as " + name, 2000);
    }
    setWindowTitle(title);
}

void YATEWindow::onDiscordVSConnectionSucceeded()
{

}

void YATEWindow::onDiscordVSConnectionFailed()
{
    qDebug() << "Failed to connect to the given lobby.";
    stopFeedback();
    QMessageBox::critical(this, "Error", "Failed to establish connection to the given lobby");
}

void YATEWindow::createTrayIcon()
{
    qDebug() << "Creating tray icon.";
    trayIcon_ = new QSystemTrayIcon(QIcon(":/yate.ico"), this);

    trayStopFeedback_ = new QAction(tr("&Stop Feedback"), this);
    trayQuit_ = new QAction(tr("&Exit"),this);
    connect(trayStopFeedback_, &QAction::triggered, this, &YATEWindow::stopFeedback);
    connect(trayQuit_, &QAction::triggered, QCoreApplication::instance(), &QCoreApplication::quit);
    QMenu *menu = new QMenu(this);
    menu->addAction(trayStopFeedback_);
    menu->addSeparator();
    menu->addAction(trayQuit_);
    trayStopFeedback_->setVisible(false);

    trayIcon_->setContextMenu(menu);

    connect(trayIcon_, &QSystemTrayIcon::activated, this, &YATEWindow::iconActivated);
}

bool YATEWindow::isLogManuallySet() const
{
    return isLogManuallySet_;
}

void YATEWindow::setIsLogManuallySet(bool newIsLogManuallySet)
{
    isLogManuallySet_ = newIsLogManuallySet;
}


void Yate::YATEWindow::showClientLiveFeedback(bool lock)
{
    qDebug() << "Starting client live feedback." << (lock? " (Locked)": " (Unlocked)");
    feedbackOverlay_ = new LiveFeedbackOverlay(nullptr, lock);
    trayStopFeedback_->setVisible(true);




    connect(discord_, &DiscordManager::onMessageFromChannel1, feedbackOverlay_, &LiveFeedbackOverlay::onUpdateMessage);
    connect(discord_, &DiscordManager::onMessageFromChannel2, feedbackOverlay_, &LiveFeedbackOverlay::onUpdateLimbs);
    connect(discord_, &DiscordManager::connectionFailed, this, &YATEWindow::onDiscordVSConnectionFailed, Qt::UniqueConnection);
    connect(feedbackOverlay_, &LiveFeedbackOverlay::onDoubleClicked, this, &YATEWindow::stopFeedback);
    connect(discord_, &DiscordManager::onLobbyDisconnect, this, &YATEWindow::stopFeedback);
    connect(feedbackOverlay_, &LiveFeedbackOverlay::onLockWindow, this, &YATEWindow::lockClientFeedbackWindow);

#ifdef DISCORD_ENABLED
    QSettings settings;
    if (settings.value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() && settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        connect(discord_, &DiscordManager::onMessageFromChannel1, discord_,&DiscordManager::setActivityDetails);
        connect(discord_, &DiscordManager::onMessageFromChannel2, discord_, &DiscordManager::setActivityState);
        connect(discord_, &DiscordManager::onMessageFromChannel3, discord_, &DiscordManager::setSquadString);
        connect(this, &YATEWindow::feedbackWindowClosed, discord_, &DiscordManager::clearActivity);
    }
#endif

    isLiveFeedbackRunning_ = true;

    feedbackOverlay_->onUpdateMessage(LIVE_FEEDBACK_DEFAULT_MSG);

    feedbackOverlay_->showNormal();
    trayIcon_->show();
    showMinimized();
    qDebug() << "Client live feedback started." << (lock? " (Locked)": " (Unlocked)");
}

void YATEWindow::on_btnLiveFeedbackVS_clicked()
{
#ifdef DISCORD_ENABLED
    qDebug() << "Client live feedback clicked.";
    QSettings settings;
    if (!settings.value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool() || !settings.value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool()) {
        qCritical() << "Discord features not enabled for client live feedback";
        QMessageBox::critical(this, "Discord Features Required", "You must enable Discord features from the settings to use this feature.");
        return;
    }
    QString lobbyId = QInputDialog::getText(this, "Host Lobby ID", "Enter the host's Lobby ID").trimmed();
    if (!lobbyId.size()) {
        qDebug() << "Returned empty Lobby Id";
        return;
    }
    qDebug() << "Connecting to Lobby ID" << lobbyId;
    if(discord_->connectTo(lobbyId)) {
        qDebug() << "Passed initial connection to Lobby";
        showClientLiveFeedback(false);
    } else {
        qCritical() << "Failed at initial connection to Lobby";
        QMessageBox::critical(this, "Error", "Failed to establish connection to lobby, double check the input value.");
    }
#else
    QMessageBox::critical(this, "Discord Features Required", "Discord features are needed but not supported by this version.");
    return;
#endif
}


void YATEWindow::on_btnCopyLobbyId_clicked()
{
    qDebug() << "Copy Lobby ID.";
    QString lobbyIdText = ui->lblLobbyId->text().trimmed();
    if (lobbyIdText.size()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(lobbyIdText);
    } else {
        qDebug() << "Lobby ID is empty.";
    }
}


void YATEWindow::on_btnCopyLobbyLink_clicked()
{
    qDebug() << "Copy Lobby Link.";
    QString lobbyIdText = ui->lblLobbyId->text().trimmed();
    if (lobbyIdText.size()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText("yate://" + lobbyIdText);
    } else {
        qDebug() << "Lobby ID is empty.";
    }
}




}
