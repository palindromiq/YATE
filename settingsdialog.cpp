#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardPaths>

#include "globals.h"
#include "yatewindow.h"

namespace Yate {
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog), settings_(new QSettings(this))
{
    ui->setupUi(this);

    reloadSettings();
    setAcceptDrops(true);
}

void SettingsDialog::setEEFilePath(QString &path)
{
    ui->lblLogFilePath->setText(path);
}

void SettingsDialog::reloadSettings()
{
    if (!settings_->value(SETTINGS_KEY_EE_LOG).isNull()) {
        QString pth = settings_->value(SETTINGS_KEY_EE_LOG).toString();
        setEEFilePath(pth);
    }
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


void SettingsDialog::on_btnSave_clicked()
{
  this->accept();
  if(!ui->lblLogFilePath->text().isEmpty()) {
      settings_->setValue(SETTINGS_KEY_EE_LOG, ui->lblLogFilePath->text().trimmed());
      YATEWindow *parentW = dynamic_cast<YATEWindow*>(parentWidget());
      parentW->setLogFilePath(ui->lblLogFilePath->text().trimmed());
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
    setEEFilePath(path);
}

}
