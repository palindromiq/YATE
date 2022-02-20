#include "timelineimagegenerator.h"
#include "globals.h"

#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QMimeData>
#include <QPainterPath>

namespace Yate {
TimelineImageGenerator::TimelineImageGenerator(QString path, NightInfo &night, QString host, QSet<QString> squad, QObject *parent)
    : QObject{parent}, path_(path), night_(night), host_(host), squad_(squad)
{
}


QImage* TimelineImageGenerator::generateImage()
{
    qDebug() << "Generating timeline image.";
    int summaryHeight = 100;
    int footerHeight = 60;
    int runNumHeight = 80;
    int numberOfRuns = 0;
    int maxLogSize = 0;
    int runWidth = 680;
    int evtHeight = 200;
    int evtLeftOffset = -70;



    for(int i = 0; i < night_.runs().size(); i++) {
        auto &set = night_.run(i);
        if (set.getNumberOfCaps() || set.getNumberOfKills()) {
            numberOfRuns++;
            maxLogSize = qMax(maxLogSize, set.eventLog().size());
        }
    }
    int imageWidth = runWidth * numberOfRuns;
    int imageHeight = evtHeight * maxLogSize + runNumHeight + summaryHeight + footerHeight;

    static QMap<LogEventType, QVector<QString>> logEvtText{
            {LogEventType::NightBegin, {"Night Started", ":/timeline/NightBegin.png"}},
            {LogEventType::DayBegin, {"Day Started", ":/timeline/DayBegin.png"}},
            {LogEventType::TeralystSpawn, {"Teralyst Spawned", ":/timeline/EidolonSpawn.png"}},
            {LogEventType::LimbBreak, {"Limb Break", ":/timeline/LimbBreak.png"}},
            {LogEventType::EidolonCapture, {"Eidolon Captured", ":/timeline/EidolonCapture.png"}},
            {LogEventType::EidolonKill, {"Eidolon Killed", ":/timeline/EidolonKill.png"}},
            {LogEventType::LootDrop, {"Loot Dropped", ":/timeline/LootDrop.png"}},
            {LogEventType::ShrineEnable, {"Shrine Enabled", ":/timeline/ShrineEnable.png"}},
            {LogEventType::ShardInsert, {"Shard Inserted", ":/timeline/ShardInsert.png"}},
            {LogEventType::ShardRemove, {"Shard Removed", ":/timeline/ShardRemove.png"}},
            {LogEventType::ShrineDisable, {"Shrine Disabled", ":/timeline/ShrineDisable.png"}},
            {LogEventType::EidolonSpawn, {"Eidolon Spawned", ":/timeline/EidolonSpawn.png"}},
            {LogEventType::HostJoin, {"Host Joinned", ":/timeline/HostJoin.png"}},
            {LogEventType::SquadJoin, {"Member Joined", ":/timeline/SquadJoin.png"}},
            {LogEventType::HostUnload, {"Host Exit", ":/timeline/HostUnload.png"}},
            {LogEventType::EidolonDespawn, {"Eidolon Despawned", ":/timeline/EidolonDespawn.png"}},
            {LogEventType::Invalid, {"Invalid", ":/timeline/Invalid.png"}}
    };

    QImage *pix = new QImage(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);

    QPainter painter(pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(QPen(QColor(SUMMARY_COLOR_TITLE), 3));
    painter.setBrush(QColor(SUMMARY_COLOR_TITLE));
    QFont font = painter.font();
    font.setFamily("Roboto Mono");
    font.setPointSize(30);
    QFontMetrics fm(font);
    painter.setFont(font);
    painter.fillRect(0, 0, pix->width(), pix->height(), QColor(SUMMARY_COLOR_BG));

    int topOffset = 0;
    int fw, fh;

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
        if (m.trimmed() != "") {
            squadList.append(m);
        }
    }
    text = squadList.join(", ");

    QString nightRes = night_.getNightResult();
    if (nightRes.size()) {
        text = text + " (" + nightRes + ")";
    }

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


    int validRunCounter = 0;
    for(int i = 0; i < night_.runs().size(); i++) {
        auto &set = night_.run(i);
        if (set.getNumberOfCaps() || set.getNumberOfKills()) {
            auto logEvts = set.eventLog();
            if (validRunCounter) {
                painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 3));
                painter.drawLine(validRunCounter * runWidth, summaryHeight, validRunCounter * runWidth, imageHeight - footerHeight);
            }
            text = "Run #" + QString::number(validRunCounter + 1);
            fm = QFontMetrics(font);
            fw = fm.horizontalAdvance(text);
            painter.drawText(validRunCounter * runWidth + (runWidth / 2.0) - (fw / 2.0) + evtLeftOffset, summaryHeight + runNumHeight/2.0 + 15, text);
            painter.setPen(QPen(QColor(TIMELINE_COLOR_ELEMENT), 6));
            int tlLineX = validRunCounter * runWidth + (runWidth / 2.0) + evtLeftOffset;
            int tlLinyYStart = summaryHeight  + runNumHeight + (0.7 * evtHeight / 2.0);
            int tlLinyYEnd = summaryHeight + (logEvts.size() - 1) * evtHeight + runNumHeight;

            painter.drawLine(tlLineX, tlLinyYStart, tlLineX, tlLinyYEnd);
            painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 3));

            if (logEvts.size()) {
                for(int j = 0; j < logEvts.size(); j++) {
                    QImage tlImg(logEvtText[logEvts[j].type()][1]);
                    tlImg = tlImg.scaled(0.7 * evtHeight, 0.7 * evtHeight);
                    int x = validRunCounter * runWidth + (runWidth / 2.0) - (tlImg.width() / 2.0) + evtLeftOffset;
                    int y = summaryHeight + j * evtHeight + runNumHeight;
                    painter.drawImage(x, y, tlImg);

                    auto timestamp = logEvts[j].timestamp();
                    float progressTime = timestamp - set.startTimestamp();
                    QString progressStr = HuntInfo::timestampToProgressString(progressTime);
                    auto evtName = logEvtText[logEvts[j].type()][0];


                    text = progressStr;
                    fm = QFontMetrics(font);
                    fw = fm.horizontalAdvance(text);
                    painter.drawText(x - fw - 15, y + (tlImg.height() / 2.0) + 15, text);
                    text = evtName;
                    painter.drawText(x + tlImg.width() + 15, y  + (tlImg.height() / 2.0) + 15, text);

                }

            }
            validRunCounter++;
        }


    }

    topOffset += evtHeight * maxLogSize + runNumHeight;

    painter.setPen(QPen(QColor(SUMMARY_COLOR_LIMBS), 2));
    font.setPointSize(20);
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
    painter.drawLine(0, imageHeight - footerHeight, imageWidth, imageHeight - footerHeight);
    painter.drawText(imageWidth/2 - fw/2, topOffset, text);

    qDebug() << "Timeline image generated.";

    return pix;
}

void TimelineImageGenerator::exportImage()
{
    auto img = generateImage();
    qDebug() << "Saving image to " << path_;
    img->save(path_);

    delete img;
    emit exportFinished(true);
}

void TimelineImageGenerator::generateAndEmit()
{
    auto img = generateImage();
    qDebug() << "Emitting generated image.";
    emit generateFinished(*img);
    delete img;
}


}
