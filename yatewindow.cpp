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

#include "globals.h"
#include "settingsdialog.h"
#include "analysiswindow.h"
#include "livefeedbackoverlay.h"
#include "eeparser.h"
#include "huntinfogenerator.h"

namespace Yate {
YATEWindow::YATEWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::YATEWindow),
      eeLogFileManuallySet_(false),
      settingsDialog_(new SettingsDialog(this)),
      feedbackOverlay_(new LiveFeedbackOverlay(this)),
      analysisWindow_(new AnalysisWindow(this)),
      parser_(nullptr),
      huntInfoGenerator_(nullptr)

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
        eeLogFileManuallySet_ = true;
    }
    QString eePath(QFileInfo(eeLogFile_).filePath());
    ui->lblLogFilePath->setText(eePath);
    settingsDialog_->setEEFilePath(eePath);
    settings.setValue(SETTINGS_KEY_EE_LOG, eePath);

    if (!eeLogFile_.exists()) {
        qWarning() << "No log file..";
    }
    createTrayIcon();
    setAcceptDrops(true);

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

    connect( parserThread, &QThread::started, parser_, &EEParser::start);
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


void YATEWindow::on_btnLiveFeedback_clicked()
{
    feedbackOverlay_->show();
    trayIcon_->show();
    hide();
}

void YATEWindow::showWindow()
{
    show();
}

void YATEWindow::stopFeedback()
{
    feedbackOverlay_->hide();
}

void YATEWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
//    case QSystemTrayIcon::Trigger:
//        qDebug() << "Trigger";
//        break;
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
//    case QSystemTrayIcon::MiddleClick:
//        qDebug() << "MiddleClick";
//        break;
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

void YATEWindow::createTrayIcon()
{
    trayIcon_ = new QSystemTrayIcon(QIcon(":/yate.ico"), this);
    trayShowYate_ = new QAction(tr("&Open"),this);
    trayStopFeedback_ = new QAction(tr("&Stop Feedback"), this);
    trayQuit_ = new QAction(tr("&Exit"),this);
    connect(trayShowYate_, &QAction::triggered, this, &YATEWindow::showWindow);
    connect(trayStopFeedback_, &QAction::triggered, this, &YATEWindow::stopFeedback);
    connect(trayQuit_, &QAction::triggered, this, &YATEWindow::exitApp);
    QMenu *menu = new QMenu(this);
    menu->addAction(trayShowYate_);
    menu->addAction(trayStopFeedback_);
    menu->addSeparator();
    menu->addAction(trayQuit_);

    trayIcon_->setContextMenu(menu);

    connect(trayIcon_, &QSystemTrayIcon::activated, this, &YATEWindow::iconActivated);
}

}
