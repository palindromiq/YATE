#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardPaths>
#include <QMessageBox>

#include "globals.h"
#include "yatewindow.h"
#include "updater.h"

namespace Yate {
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog), settings_(new QSettings(this))
{
    qDebug() << "Initalized Settings window.";
    ui->setupUi(this);

    reloadSettings();
    setAcceptDrops(true);
    ui->lblVersion->setText("YATE " + Updater::getInstance()->getVersion());
    ui->lblWebsite->setText("<a style=\"color: rgb(255, 255, 255);\" href=\"" + SETTINGS_WEBSITE_HTTPS + "\">" + SETTINGS_WEBSITE + "</a>");
    ui->lblWebsite->setTextFormat(Qt::RichText);
    ui->lblWebsite->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->lblWebsite->setOpenExternalLinks(true);
    qDebug() << "Settings window initialized.";
}

void SettingsDialog::setEEFilePath(QString &path)
{
    ui->lblLogFilePath->setText(path);
}

void SettingsDialog::reloadSettings()
{
    qDebug() << "Reloading Settings window.";
    if (!settings_->value(SETTINGS_KEY_EE_LOG).isNull()) {
        QString pth = settings_->value(SETTINGS_KEY_EE_LOG).toString();
        setEEFilePath(pth);
    }
    if (!settings_->value(SETTINGS_KEY_FEEDBACK_FONT).isNull()) {
        int font = settings_->value(SETTINGS_KEY_FEEDBACK_FONT).toInt();
        ui->spnFeedbackFont->setValue(font);
    }
    bool showLimbsSummary = true;
    if (!settings_->value(SETTINGS_KEY_SHOW_LIMBS).isNull()) {
        showLimbsSummary = settings_->value(SETTINGS_KEY_SHOW_LIMBS) == "true";
        ui->chkShowLimbs->setChecked(showLimbsSummary);
    }
    if (!settings_->value(SETTINGS_KEY_SHOW_LIMBS_AFTER_LAST).isNull()) {
        bool showLimbsSummaryAfterLast = settings_->value(SETTINGS_KEY_SHOW_LIMBS_AFTER_LAST) == "true";
        ui->chkShowLimbsAfterLast->setChecked(showLimbsSummaryAfterLast);
    }
    ui->chkAutoUpdate->setChecked(settings_->value(SETTINGS_KEY_UPDATE_ON_STARTUP, true).toBool());
    ui->spnLimbsPrec->setValue(settings_->value(SETTINGS_KEY_LIMBS_PREC, SETTINGS_LIMBS_PREC_DEFAULT).toInt());
    ui->chkLockFeedbackButton->setChecked(settings_->value(SETTINGS_KEY_LOCK_FEEDBACK_BTN, true).toBool());
    ui->chkStreamer->setChecked(settings_->value(SETTINGS_KEY_STREAMER_MODE, false).toBool());
    ui->chkDiscord->setChecked(settings_->value(SETTINGS_KEY_DISCORD_FEATURES, true).toBool());
    ui->chkDiscordActivity->setChecked(settings_->value(SETTINGS_KEY_DISCORD_ACTIVITY, true).toBool());
    ui->chkClientsServer->setChecked(settings_->value(SETTINGS_KEY_DISCORD_NETWORKING, true).toBool());

    on_chkShowLimbs_toggled(showLimbsSummary);
}

void SettingsDialog::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void SettingsDialog::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        setEEFilePath(fileName);
        break;
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::onUpdateStatusUpdate(QString status)
{
    ui->lblUpdateStatus->setText(status);
}

void SettingsDialog::onUpdateAvailable()
{
    auto result = QMessageBox::question(this, "Update YATE", "A new version (" + Updater::getInstance(0)->latestVersion() + ") is available, do you want to update to the latest version?",
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::NoToAll);
    if (result == QMessageBox::Yes) {
        Updater::getInstance(0)->startUpdate();
    } else if (result == QMessageBox::NoToAll) {
        settings_->setValue(SETTINGS_KEY_UPDATE_ON_STARTUP, false);
        ui->chkAutoUpdate->setChecked(false);
    }
}

void SettingsDialog::onUpdaterError(QString err) {
    QMessageBox::critical(this, "Update Error", err);
}

void SettingsDialog::lockUpdateBtn(bool busy)
{
    ui->btnCheckUpdates->setEnabled(!busy);
}


void SettingsDialog::on_btnSave_clicked()
{
  qDebug() << "Saving new settings.";
  this->accept();
  YATEWindow *parentW = dynamic_cast<YATEWindow*>(parentWidget());
  if(!ui->lblLogFilePath->text().isEmpty()) {
      settings_->setValue(SETTINGS_KEY_EE_LOG, ui->lblLogFilePath->text().trimmed());
      if (parentW && !parentW->isLogManuallySet()) {
        parentW->setLogFilePath(ui->lblLogFilePath->text().trimmed());
      }
  }
  settings_->setValue(SETTINGS_KEY_FEEDBACK_FONT, ui->spnFeedbackFont->value());
  if(ui->chkShowLimbs->isChecked()) {
      settings_->setValue((SETTINGS_KEY_SHOW_LIMBS), "true");
  } else {
      settings_->setValue((SETTINGS_KEY_SHOW_LIMBS), "false");
  }
  if(ui->chkShowLimbsAfterLast->isChecked()) {
      settings_->setValue((SETTINGS_KEY_SHOW_LIMBS_AFTER_LAST), "true");
  } else {
      settings_->setValue((SETTINGS_KEY_SHOW_LIMBS_AFTER_LAST), "false");
  }
  settings_->setValue(SETTINGS_KEY_LIMBS_PREC, ui->spnLimbsPrec->value());
  settings_->setValue(SETTINGS_KEY_LOCK_FEEDBACK_BTN, ui->chkLockFeedbackButton->isChecked());
  settings_->setValue(SETTINGS_KEY_STREAMER_MODE, ui->chkStreamer->isChecked());
  settings_->setValue(SETTINGS_KEY_UPDATE_ON_STARTUP, ui->chkAutoUpdate->isChecked());
  settings_->setValue(SETTINGS_KEY_DISCORD_FEATURES, ui->chkDiscord->isChecked());
  settings_->setValue(SETTINGS_KEY_DISCORD_ACTIVITY, ui->chkDiscordActivity->isChecked());
  settings_->setValue(SETTINGS_KEY_DISCORD_NETWORKING, ui->chkClientsServer->isChecked());
  if (parentW) {
      parentW->refreshDiscordSettings();
  }
}


void SettingsDialog::on_btnCancel_clicked()
{
  this->reject();
}


void SettingsDialog::on_btnBrowseLogPath_clicked()
{
    QString newPath = QFileDialog::getOpenFileName(this, tr("EE.log file"), QFileInfo( ui->lblLogFilePath->text()).dir().absolutePath(), tr("Log File (*.log)"));
    if(newPath.length()) {
        setEEFilePath(newPath);
    }
}


void SettingsDialog::on_btnResetFeedback_clicked()
{
  settings_->remove(SETTINGS_KEY_FEEDBACK_POS_X);
  settings_->remove(SETTINGS_KEY_FEEDBACK_POS_Y);
}


void SettingsDialog::on_btnDefaultPath_clicked()
{
    QString appDataPath;
    QStringList appDataLocations = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    if (!appDataLocations.length()) {
        appDataPath = QDir::homePath() + QDir::separator() + "AppData" + QDir::separator() + "Local";
    } else {
        appDataPath = appDataLocations.first();
        appDataPath = QFileInfo(appDataPath).dir().absolutePath(); // Up one directory
        appDataPath = QFileInfo(appDataPath).dir().absolutePath(); // Up one directory
    }
    QString path = QDir(appDataPath).filePath(PATH_EE_LOG_RELATIVE);
    qDebug() << "Updating log file path to " << path;
    setEEFilePath(path);
}


void SettingsDialog::on_chkShowLimbs_toggled(bool checked)
{
    ui->chkShowLimbsAfterLast->setEnabled(checked);
    ui->spnLimbsPrec->setEnabled(checked);
}


void SettingsDialog::on_btnCheckUpdates_clicked()
{
  qDebug() << "Checking for updates (from Settings).";
  emit checkForUpdate();
}


void SettingsDialog::on_chkDiscord_stateChanged(int)
{
  qDebug() << "Discord features set to " << ui->chkDiscord->isChecked();
  ui->chkDiscordActivity->setEnabled(ui->chkDiscord->isChecked());
  ui->chkClientsServer->setEnabled(ui->chkDiscord->isChecked());
}


}
