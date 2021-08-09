#ifndef LIVEFEEDBACKOVERLAY_H
#define LIVEFEEDBACKOVERLAY_H

#include <QMainWindow>

namespace Ui {
class LiveFeedbackOverlay;
}

class QSettings;

namespace Yate {
class LiveFeedbackOverlay : public QMainWindow
{
    Q_OBJECT

public:
    explicit LiveFeedbackOverlay(QWidget *parent = nullptr, bool locked = false);
    ~LiveFeedbackOverlay();

    void refreshSize();
    
private slots:
    void mousePressEvent(QMouseEvent *evt);
    void mouseMoveEvent(QMouseEvent *evt);
    void mouseReleaseEvent(QMouseEvent *evt);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void on_btnLockFeedback_clicked();

public slots:
    void onUpdateMessage(QString msg);
    void onUpdateLimbs(QString msg);

signals:
    void onDoubleClicked();
    void onLockWindow();

protected:
    void showEvent(QShowEvent *evt);

private:
    Ui::LiveFeedbackOverlay *ui;
    QPointF oldPos_;
    bool isMoving_;
    QSettings *settings_;
    bool isLocked_;
};
}
#endif // LIVEFEEDBACKOVERLAY_H
