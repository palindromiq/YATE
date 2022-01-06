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
#include <iostream>

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
void Yate::YATEWindow::initDiscord()
{
    discordThread_ = new QThread;
    discord_ = new DiscordManager;
    discord_->moveToThread(discordThread_);
    connect(discordThread_, &QThread::started, discord_, &DiscordManager::start);
    connect(discordThread_, &QThread::finished, discordThread_, &QThread::deleteLater);
    connect(this, &YATEWindow::discordStart, discord_, &DiscordManager::start);
    connect(this, &YATEWindow::discordStop, discord_, &DiscordManager::stop);
    discordThread_->start();
}

YATEWindow::YATEWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::YATEWindow),
      settingsDialog_(new SettingsDialog(this)),
      feedbackOverlay_(nullptr),
      analysisWindow_(new AnalysisWindow(this)),
      parser_(nullptr),
      huntInfoGenerator_(nullptr),
      isLogManuallySet_(false),
      isLiveFeedbackRunning_(false)

{
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
//    connect(updater, &Updater::onBusyUpdate, this, &YATEWindow::onUpdaterBusy);
    if (settings.value(SETTINGS_KEY_UPDATE_ON_STARTUP, true).toBool()) {
        emit checkForUpdate();
    }
#ifdef DISCORD_ENABLED
    if (settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        initDiscord();
    } else {
        discordThread_ = nullptr;
        discord_ = nullptr;
    }

#endif

}


void YATEWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void YATEWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        ui->lblLogFilePath->setText(fileName);
        eeLogFile_.setFileName(fileName);
        isLogManuallySet_ = true;
        break;
    }
}

YATEWindow::~YATEWindow()
{
    delete ui;
}


void YATEWindow::on_btnSettings_clicked()
{
    settingsDialog_->reloadSettings();
    settingsDialog_->exec();
}


void YATEWindow::on_btnEditLogPath_clicked()
{
    QString newPath = QFileDialog::getOpenFileName(this, tr("EE.log file"), QFileInfo(eeLogFile_.fileName()).dir().absolutePath(), tr("Log File (*.log)"));
    if(newPath.length()) {
        ui->lblLogFilePath->setText(newPath);
        eeLogFile_.setFileName(newPath);
        isLogManuallySet_ = true;
    }
}


void YATEWindow::on_btnShrineWater_clicked()
{
    disableUI();
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
    if (settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        connect(huntInfoGenerator_, &HuntInfoGenerator::onHuntStateChanged, discord_, &DiscordManager::setActivityDetails);
        connect(huntInfoGenerator_, &HuntInfoGenerator::onLimbsChanged, discord_, &DiscordManager::setActivityState);
        connect(huntInfoGenerator_, &HuntInfoGenerator::onSquadChanged, discord_, &DiscordManager::setSquad);
        connect(this, &YATEWindow::feedbackWindowClosed, discord_, &DiscordManager::clearActivity);
    }
#endif

    parserThread->start();
    isLiveFeedbackRunning_ = true;

    feedbackOverlay_->onUpdateMessage(LIVE_FEEDBACK_DEFAULT_MSG);

    feedbackOverlay_->showNormal();
    trayIcon_->show();
    showMinimized();
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
            if(isLiveFeedbackRunning_) {
                stopFeedback();
            }
        }
    }
}

void YATEWindow::stopFeedback()
{
    trayStopFeedback_->setVisible(false);
    feedbackOverlay_->close();
    trayIcon_->hide();
    isLiveFeedbackRunning_ = false;
    showNormal();
    emit feedbackWindowClosed();
}

void YATEWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
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
    ui->btnShrineWater->setEnabled(false);
    ui->btnSettings->setEnabled(false);
}

void YATEWindow::enableUI()
{
    ui->btnEditLogPath->setEnabled(true);
    ui->btnLiveFeedback->setEnabled(true);
    ui->btnShrineWater->setEnabled(true);
    ui->btnSettings->setEnabled(true);
}

void YATEWindow::onParserStarted()
{

}

void YATEWindow::onParserFinished()
{
    enableUI();
    // Set analysisWindow_ model
    analysisWindow_->setHunt(huntInfoGenerator_->huntInfo());
    analysisWindow_->show();
}

void YATEWindow::onParserError(QString err)
{
    qCritical() << err;
    enableUI();
}

void YATEWindow::lockFeedbackWindow()
{
    feedbackOverlay_->close();
    showLiveFeedback(true);
}

void YATEWindow::setLogFilePath(QString path)
{
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
    QSettings settings;
    if (settings.value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool()) {
        if (!discord_) {
            initDiscord();
        } else {
            emit discordStart();
        }

    } else {
        emit discordStop();
    }
#endif
}

void YATEWindow::createTrayIcon()
{
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

}
