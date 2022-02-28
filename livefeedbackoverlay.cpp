#include "livefeedbackoverlay.h"
#include "ui_livefeedbackoverlay.h"

#include <QMouseEvent>
#include <QSettings>
#include <QDateTime>

#include "globals.h"

namespace Yate {
LiveFeedbackOverlay::LiveFeedbackOverlay(QWidget *parent, bool locked) :
    QMainWindow(parent),
    ui(new Ui::LiveFeedbackOverlay), isMoving_(false),settings_(new QSettings(this)), isLocked_(false)
{
    qDebug() << "Initializing feedback overlay screen.";
    ui->setupUi(this);
    setStyleSheet("background:transparent");
    ui->lblFeedback->setStyleSheet("background:rgba(0,0,0,0.7); font-weight: 500;");
    ui->lblLimbs->setStyleSheet("background:rgba(0,0,0,0.7)");
    ui->btnLockFeedback->setVisible(settings_->value(SETTINGS_KEY_LOCK_FEEDBACK_BTN, true).toBool());


    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
    if (locked) {
        ui->btnLockFeedback->setVisible(false);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    } else {
        float lockScaleFactor = settings_->value(SETTINGS_KEY_FEEDBACK_FONT, SETTINGS_FEEDBACK_FONT_DEFAULT).toInt() / 8.0;
        ui->btnLockFeedback->setIconSize(QSize(16 * lockScaleFactor, 16 * lockScaleFactor));
    }

    if (settings_->value(SETTINGS_KEY_STREAMER_MODE, false).toBool()) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    } else {
        qDebug() << "Using Streamer Mode window flags";
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
    }


    if(!settings_->value(SETTINGS_KEY_FEEDBACK_POS_X).isNull()) {
        int xpos = settings_->value(SETTINGS_KEY_FEEDBACK_POS_X).toInt();
        int ypos = settings_->value(SETTINGS_KEY_FEEDBACK_POS_Y).toInt();
        move(xpos, ypos);
    }

    ui->lblFeedback->setMouseTracking(true);
    ui->lblFeedback->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lblLimbs->setMouseTracking(true);
    ui->lblLimbs->setAttribute(Qt::WA_TransparentForMouseEvents);
    qDebug() << "Initialized feedback overlay screen.";
}

void Yate::LiveFeedbackOverlay::refreshSize()
{
    int fontSz = settings_->value(SETTINGS_KEY_FEEDBACK_FONT, SETTINGS_FEEDBACK_FONT_DEFAULT).toInt();
    bool showLimbs = settings_->value(SETTINGS_KEY_SHOW_LIMBS, "true") == "true";
    bool showLimbsLabel = showLimbs && ui->lblLimbs->text().trimmed() != "";
    if (showLimbsLabel) {
        ui->lblLimbs->show();
    } else {
        ui->lblLimbs->hide();
    }

    QFont lblFont = ui->lblFeedback->font();
    QFont lblLimbsFont = ui->lblFeedback->font();
    lblFont.setPointSize(fontSz);
    lblLimbsFont.setPointSize(fontSz);
    ui->lblFeedback->setFont(lblFont);
    ui->lblLimbs->setFont(lblLimbsFont);

    ui->btnLockFeedback->setSizePolicy (ui->btnLockFeedback->sizePolicy().horizontalPolicy(), QSizePolicy::Ignored);
    float h = ui->lblFeedback->height();
    if (ui->lblLimbs->isVisible()) {
        h += ui->lblLimbs->height();
    }
    ui->btnLockFeedback->setFixedHeight(h);
    return;

}

void LiveFeedbackOverlay::showEvent(QShowEvent *evt) {
    refreshSize();
    QMainWindow::showEvent(evt);
}

void LiveFeedbackOverlay::mousePressEvent(QMouseEvent *evt)
{
    oldPos_ = evt->globalPosition();
    isMoving_ = true;
    QMainWindow::mousePressEvent(evt);
}

void LiveFeedbackOverlay::mouseDoubleClickEvent(QMouseEvent *evt) {
    qDebug() << "Live feedback double clicked.";
    emit onDoubleClicked();
    QMainWindow::mouseDoubleClickEvent(evt);
}

void LiveFeedbackOverlay::mouseReleaseEvent(QMouseEvent *evt)
{
    oldPos_ = evt->globalPosition();
    isMoving_ = false;
    QMainWindow::mouseReleaseEvent(evt);
}

void LiveFeedbackOverlay::onUpdateMessage(QString msg)
{
    ui->lblFeedback->setText("" + msg);
    refreshSize();
}

void LiveFeedbackOverlay::onUpdateLimbs(QString msg)
{
    if (msg.trimmed() == "") {
        ui->lblLimbs->hide();
    } else {
        ui->lblLimbs->show();
    }
    ui->lblLimbs->setText("" + msg + " ");
    refreshSize();
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
    QMainWindow::mouseMoveEvent(evt);
}

LiveFeedbackOverlay::~LiveFeedbackOverlay()
{
    delete ui;
}


void LiveFeedbackOverlay::on_btnLockFeedback_clicked()
{
  if(!isLocked_) {
    qDebug() << "Lock button pressed.";
    emit onLockWindow();
  }
}

}
