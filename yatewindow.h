#ifndef YATEWINDOW_H
#define YATEWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDir>
#include <QString>
#include <QSystemTrayIcon>





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

class YATEWindow : public QMainWindow
{
    Q_OBJECT

public:
    YATEWindow(QWidget *parent = nullptr);
    ~YATEWindow();

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

public slots:
    void setLogFilePath(QString path);

signals:
    void exitFeebackOverlay();

private:
    void createTrayIcon();

    Ui::YATEWindow *ui;
    QFile eeLogFile_;
    bool eeLogFileManuallySet_;
    SettingsDialog *settingsDialog_;
    LiveFeedbackOverlay *feedbackOverlay_;
    AnalysisWindow *analysisWindow_;
    QSystemTrayIcon *trayIcon_;
    QAction *trayShowYate_;
    QAction *trayStopFeedback_;
    QAction *trayQuit_;
    EEParser *parser_;
    HuntInfoGenerator *huntInfoGenerator_;


};
}
#endif // YATEWINDOW_H
