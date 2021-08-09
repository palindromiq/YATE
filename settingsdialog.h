#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>


namespace Ui {
class SettingsDialog;
}
class QSettings;

namespace Yate {
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setEEFilePath(QString &path);

    void reloadSettings();

    ~SettingsDialog();

private slots:
    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_btnBrowseLogPath_clicked();

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    void on_btnResetFeedback_clicked();

    void on_btnDefaultPath_clicked();


    void on_chkShowLimbs_toggled(bool checked);

private:
    Ui::SettingsDialog *ui;
    QSettings *settings_;
};
}

#endif // SETTINGSDIALOG_H
