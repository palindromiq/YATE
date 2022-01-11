#include "huntimagegenerator.h"
#include "globals.h"

#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QMimeData>

namespace Yate {
HuntImageGenerator::HuntImageGenerator(QString path, NightInfo &night, QString host, QSet<QString> squad, QObject *parent)
    : QObject{parent}, path_(path), night_(night), host_(host), squad_(squad)
{
}


QImage* HuntImageGenerator::generateImage()
{
    qDebug() << "Generating summary image.";
    int numSets = night_.validRunCount();
    int setHeight = 216;
    int capHeight = setHeight / 3;
    int summaryHeight = capHeight / 1.5;
    int footerHeight = capHeight / 2;
    int imageWidth = 1200;
    int imageHeight = numSets * setHeight + summaryHeight + footerHeight;

    QImage *pix = new QImage(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);

    QPainter painter(pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 3));
    painter.setBrush(QColor(SUMMARY_COLOR_LIMBS));
    QFont font = painter.font();
//    font.setFamily("Courier New");
    font.setFamily("Roboto Mono");
    font.setPointSize(18);
    QFontMetrics fm(font);
    painter.setFont(font);
    painter.fillRect(0, 0, pix->width(), pix->height(), QColor(SUMMARY_COLOR_BG));

    int topOffset = 0;
    int fw, fh;
    int textYOffset = 10;
    int spaceWidth = QFontMetrics(painter.font()).horizontalAdvance(" ");
    QString text;

    text = "Squad";
    font.setBold(true);
    fm = QFontMetrics(font);
    painter.setFont(font);
    int xpos = 5;
    int ypos = topOffset + summaryHeight / 2 + 4;
    painter.setPen(QPen(QColor(SUMMARY_COLOR_TITLE), 2));
    painter.drawText(xpos, ypos, text);

    xpos += fm.horizontalAdvance(text) + 2 * spaceWidth;

    QStringList squadList;
    squadList.append(host_);
    for(auto &m: squad_) {
        squadList.append(m);
    }
    text = squadList.join(", ");
    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 2));
    painter.drawText(xpos, ypos, text);

    painter.setPen(QPen(QColor(SUMMARY_COLOR_AVERAGE), 2));
    QString txtAvg = night_.getAverage();
    text = txtAvg;
    painter.drawText(imageWidth - fm.horizontalAdvance(text) - 5, ypos, text);

    painter.setPen(QPen(QColor(SUMMARY_COLOR_TITLE), 2));
    text = "Average";
    painter.drawText(imageWidth - fm.horizontalAdvance(txtAvg) - spaceWidth - fm.horizontalAdvance(text) - 5, ypos, text);


    topOffset += summaryHeight;
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 3));
    painter.drawLine(0, topOffset, imageWidth, topOffset);

    for(int i = 0; i < night_.runs().size(); i++) {
        auto &set = night_.run(i);
        if (set.getNumberOfCaps() || set.getNumberOfKills()) {
            for (int j = 0; j < 3; j++) {
                auto cap = set.capInfoByIndex(j);
                if (cap.valid()) {
                    text = HuntInfo::eidolonName(j, true);
                    fm = QFontMetrics(painter.font());
                    xpos = 5;

                    fw = fm.horizontalAdvance(text);
                    int ypos = topOffset + capHeight/2 + textYOffset;
                    painter.setPen(QPen(QColor(SUMMARY_COLOR_TITLE), 2));
                    painter.drawText(xpos, ypos, text);

                    xpos += fw + 2 * spaceWidth;
                    QStringList limbsVals;
                    QString ws = FORMAT_NUMBER(cap.waterShield());
                    while (ws.size() < 6) {
                        ws = ws + " ";
                    }
                    limbsVals.append(ws);
                    for(auto &l: cap.limbBreaks()) {
                        QString limbStr = FORMAT_NUMBER(l);
                        limbsVals.append(limbStr);
                    }


                    text = limbsVals.join("  ");
                    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 2));
                    painter.drawText(xpos, ypos, text);
                    if (cap.result() == CapState::InComplete || cap.result() == CapState::Spawned) {
                        text = "Incomplete";
                    } else {
                        text =  HuntInfo::timestampToProgressString(cap.lastLimbProgressTime()) + " [" + FORMAT_NUMBER(cap.capshotTime()) + "] (" +  (cap.result() == CapState::Capture? "Captured": "Killed") + ")";
                    }
                    font.setFamily("Robot Mono Medium");
                    font.setBold(true);
                    painter.setFont(font);
                    painter.setPen(QPen(QColor(SUMMARY_COLOR_LAST_LIMB), 2));
                    fm = QFontMetrics(font);
                    fw = fm.horizontalAdvance(text);
                    painter.drawText(imageWidth - fw - 5, ypos, text);
                    font.setFamily("Robot Mono");
                    font.setBold(false);
                    fm = QFontMetrics(font);
                    painter.setFont(font);

                }

                topOffset += capHeight;
                if (j != 2) {
                    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 1));
                    painter.drawLine(0, topOffset, imageWidth, topOffset);
                }
            }
            painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 3));
            painter.drawLine(0, topOffset, imageWidth, topOffset);

        }

    }

    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 2));
    font.setPointSize(14);
    painter.setFont(font);
    QString ver = QCoreApplication::applicationVersion();
    if (ver.count(".") == 3) {
        auto verDiv = ver.split(".");
        verDiv.pop_back();
        ver = verDiv.join(".");
    }
    text = "Generated by YATE " + ver + " (" + QDate::currentDate().toString("MMM dd, yyyy") + ")";
    fm = QFontMetrics(font);
    fw = fm.horizontalAdvance(text);
    fh = fm.height();
    topOffset += footerHeight - fh/2;

    painter.drawText(imageWidth/2 - fw/2, topOffset, text);

    qDebug() << "Summary image generated.";

    return pix;
}

void HuntImageGenerator::exportImage()
{
    auto img = generateImage();
    qDebug() << "Saving image to " << path_;
    img->save(path_);

    delete img;
    emit exportFinished(true);
}

void HuntImageGenerator::generateAndEmit()
{
    auto img = generateImage();
    qDebug() << "Emitting generated image.";
    emit generateFinished(*img);
    delete img;
}


}
