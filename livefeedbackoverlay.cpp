#include "livefeedbackoverlay.h"
#include "ui_livefeedbackoverlay.h"

#include <QMouseEvent>
#include <QSettings>
#include <QDateTime>

#include "globals.h"

namespace Yate {
LiveFeedbackOverlay::LiveFeedbackOverlay(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LiveFeedbackOverlay), isMoving_(false),settings_(new QSettings(this))
{
    ui->setupUi(this);
    setStyleSheet("background:transparent");
    ui->lblFeedback->setStyleSheet("background:rgba(0,0,0,0.7)");
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    if(!settings_->value(SETTINGS_KEY_FEEDBACK_POS_X).isNull()) {
        int xpos = settings_->value(SETTINGS_KEY_FEEDBACK_POS_X).toInt();
        int ypos = settings_->value(SETTINGS_KEY_FEEDBACK_POS_Y).toInt();
        move(xpos, ypos);
    }

}

void LiveFeedbackOverlay::mousePressEvent(QMouseEvent *evt)
{
    oldPos_ = evt->globalPosition();
    isMoving_ = true;
}

void LiveFeedbackOverlay::mouseDoubleClickEvent(QMouseEvent *evt) {
    emit onDoubleClicked();
    QMainWindow::mouseDoubleClickEvent(evt);
}

void LiveFeedbackOverlay::mouseReleaseEvent(QMouseEvent *evt)
{
    oldPos_ = evt->globalPosition();
    isMoving_ = false;
}

void LiveFeedbackOverlay::onUpdateMessage(QString msg)
{
//    ui->lblFeedback->setText(QString("[") + QDateTime::currentDateTime().toString("HH:mm:ss") + "] " + msg);
    ui->lblFeedback->setText(" " + msg);
}

void LiveFeedbackOverlay::mouseMoveEvent(QMouseEvent *evt)
{
    if(isMoving_) {
        const QPointF delta = evt->globalPosition() - oldPos_;
        QPoint newPos(x()+delta.x(), y()+delta.y());
        move(newPos);
        oldPos_ = evt->globalPosition();
        settings_->setValue(SETTINGS_KEY_FEEDBACK_POS_X, newPos.x());
        settings_->setValue(SETTINGS_KEY_FEEDBACK_POS_Y, newPos.y());
    }
}

LiveFeedbackOverlay::~LiveFeedbackOverlay()
{
    delete ui;
}
}
