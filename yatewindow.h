#ifndef YATEWINDOW_H
#define YATEWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDir>
#include <QString>
#include <QSystemTrayIcon>

#include "globals.h"





QT_BEGIN_NAMESPACE
namespace Ui { class YATEWindow; }

class QAction;

QT_END_NAMESPACE




namespace Yate {

class SettingsDialog;
class LiveFeedbackOverlay;
class AnalysisWindow;
class EEParser;
class HuntInfoGenerator;
class DiscordManager;

class YATEWindow : public QMainWindow
{
    Q_OBJECT

public:
    YATEWindow(bool clientVersion = false, QWidget *parent = nullptr);
    ~YATEWindow();

    bool isLogManuallySet() const;
    void setIsLogManuallySet(bool newIsLogManuallySet);
    void showLiveFeedback(bool lock);

protected:
    void changeEvent(QEvent* e);

private slots:
    void on_btnSettings_clicked();

    void on_btnEditLogPath_clicked();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    void on_btnShrineWater_clicked();

    void on_btnLiveFeedback_clicked();

    void showWindow();
    void stopFeedback();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void exitApp();

    void disableUI();
    void enableUI();

    void onParserStarted();
    void onParserFinished();
    void onParserError(QString);

    void lockFeedbackWindow();

public slots:
    void setLogFilePath(QString path);
    void onUpdaterBusy(bool busy);
    void refreshDiscordSettings();

signals:
    void exitFeebackOverlay();
    void checkForUpdate();
    void feedbackWindowClosed();
    void discordStop();
    void discordStart();
    void discordClearActivity();

private:
    void createTrayIcon();


    Ui::YATEWindow *ui;
    QFile eeLogFile_;
    SettingsDialog *settingsDialog_;
    LiveFeedbackOverlay *feedbackOverlay_;
    AnalysisWindow *analysisWindow_;
    QSystemTrayIcon *trayIcon_;
    QAction *trayShowYate_;
    QAction *trayStopFeedback_;
    QAction *trayQuit_;
    EEParser *parser_;
    HuntInfoGenerator *huntInfoGenerator_;
    bool isLogManuallySet_;
    bool isLiveFeedbackRunning_;
    bool clientVersion_;

#ifdef DISCORD_ENABLED
    DiscordManager *discord_;
    QThread *discordThread_;
#endif

    void initDiscord();
};
}
#endif // YATEWINDOW_H
