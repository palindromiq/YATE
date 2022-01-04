#include "analysiswindow.h"
#include "ui_analysiswindow.h"
#include "analysisviewmodel.h"
#include "huntimagegenerator.h"
#include "huntinfo.h"
#include <QFileDialog>

namespace Yate {
AnalysisWindow::AnalysisWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::AnalysisWindow),
    model_(nullptr), hunt_(nullptr), selectedNight_(-1)
{
    ui->setupUi(this);
    unhighlightNight();
//    HuntInfo *huntInfo = new HuntInfo;
//    model_ = new AnalysisViewModel(huntInfo, this);
//    ui->treAnalysisView->setModel(model_);
//    ui->treAnalysisView->setColumnWidth(0,  width() / 3.0);
//    for(int i = 0; i < model_->rowCount(); i++) {
//        ui->treAnalysisView->expand(model_->index(i, 0));
//    }

}



AnalysisWindow::~AnalysisWindow()
{
    delete ui;
}

HuntInfo *AnalysisWindow::hunt() const
{
    return hunt_;
}

void AnalysisWindow::setHunt(HuntInfo *newHunt)
{
    hunt_ = newHunt;
    model_ = new AnalysisViewModel(hunt_, this);
    ui->treAnalysisView->setModel(model_);
    for(int i = 0; i < model_->rowCount(); i++) {
        ui->treAnalysisView->expand(model_->index(i, 0));
    }
    if (hunt_->nightCount() == 1) {
        highlightNight(0);
    } else {
        unhighlightNight();
    }
}

HuntInfo *AnalysisWindow::createDummyHunt()
{
    HuntInfo *huntInfo = new HuntInfo;
    NightInfo night1;
    RunInfo night1Run1;
    CapInfo night1Run1TerryCap;
    night1Run1TerryCap.setEidolon(Eidolon::Teralyst);
    night1Run1TerryCap.setCapshotTime(0.123);
    night1Run1TerryCap.setResult(CapState::Capture);
    night1Run1TerryCap.setShrineActivationTime(11.0);
    night1Run1TerryCap.setShrineTime(10.9);
    night1Run1TerryCap.setSpawnDelay(0.3);
    night1Run1TerryCap.setWaterShield(32.1);
    night1Run1TerryCap.setValid(true);

    CapInfo night1Run1GarryCap;
    night1Run1GarryCap.setEidolon(Eidolon::Gantulyst);
    night1Run1GarryCap.setCapshotTime(0.121);
    night1Run1GarryCap.setResult(CapState::Capture);
    night1Run1GarryCap.setShrineActivationTime(11.1);
    night1Run1GarryCap.setShrineTime(10.2);
    night1Run1GarryCap.setSpawnDelay(0.4);
    night1Run1GarryCap.setWaterShield(2.7);
    night1Run1GarryCap.setValid(true);

    CapInfo night1Run1HarryCap(night1Run1GarryCap);
    night1Run1HarryCap.setEidolon(Eidolon::Hydrolyst);

    night1Run1.setTeralystCapInfo(night1Run1TerryCap);
    night1Run1.setGantulystCapInfo(night1Run1GarryCap);
    night1Run1.setHydrolystCapInfo(night1Run1HarryCap);

    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);

    huntInfo->addNight(night1);
    huntInfo->addNight(night1);

    return huntInfo;

}


void AnalysisWindow::on_btnClose_clicked()
{
  this->close();
}


void AnalysisWindow::on_treAnalysisView_clicked(const QModelIndex &index)
{
  auto trav = index;
  while(trav.parent().isValid()) {
      trav = trav.parent();
  }
  if (trav.isValid()) {
      highlightNight(trav.row());
  } else {
      unhighlightNight();
  }

}

void AnalysisWindow::highlightNight(int night)
{
    selectedNight_ = night;
    ui->btnExport->setEnabled(true);
    ui->btnExport->setToolTip("");
}

void AnalysisWindow::unhighlightNight()
{
    selectedNight_ = -1;
    ui->btnExport->setEnabled(false);
    ui->btnExport->setToolTip("Select a night analysis to export.");
}


void AnalysisWindow::on_btnExport_clicked()
{
  if (selectedNight_ == -1 || (selectedNight_ >= hunt_->nightCount())) {
      unhighlightNight();
      return;
  }
  NightInfo &night = hunt_->night(selectedNight_);
  QString savePath = QFileDialog::getSaveFileName(this, "Export Hunt Summary", "Hunt_" + QDateTime::currentDateTime().toString("MM_dd_yy_hh") + ".png", ".png");

  if (!savePath.size()) {
      return;
  }

  if (!savePath.endsWith(".png")) {
      if (!savePath.endsWith(".")) {
          savePath = savePath + ".";
      }
      savePath = savePath + "png";
  }

  HuntImageGenerator gen(night, hunt_->host(), hunt_->squad());
//  qDebug() << gen.saveImage(savePath);
}




}
