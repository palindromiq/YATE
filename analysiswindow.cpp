#include "analysiswindow.h"
#include "ui_analysiswindow.h"
#include "analysisviewmodel.h"
#include "huntinfo.h"

namespace Yate {
AnalysisWindow::AnalysisWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnalysisWindow)
{
    ui->setupUi(this);
    HuntInfo *huntInfo = createDummyHunt();
    model_ = new AnalysisViewModel(huntInfo, this);
    ui->treAnalysisView->setModel(model_);

}



AnalysisWindow::~AnalysisWindow()
{
    delete hunt_;
    delete ui;
}

HuntInfo *AnalysisWindow::hunt() const
{
    return hunt_;
}

void AnalysisWindow::setHunt(HuntInfo *newHunt)
{
    hunt_ = newHunt;
}

HuntInfo *AnalysisWindow::createDummyHunt()
{
    HuntInfo *huntInfo = new HuntInfo;
    NightInfo night1;
    RunInfo night1Run1;
    CapInfo night1Run1TerryCap;
    night1Run1TerryCap.setEidolon(Eidolon::Terralyst);
    night1Run1TerryCap.setCapshotTime(0.123);
    night1Run1TerryCap.setHealingPhaseTime(12.0);
    night1Run1TerryCap.setResult(CapState::Capture);
    night1Run1TerryCap.setShrineActivationTime(11.0);
    night1Run1TerryCap.setShrineTime(10.9);
    night1Run1TerryCap.setSpawnDelay(0.3);
    night1Run1TerryCap.setWaterShield(32.1);
    night1Run1TerryCap.setSpawned(true);
    night1Run1TerryCap.setValid(true);

    CapInfo night1Run1GarryCap;
    night1Run1GarryCap.setEidolon(Eidolon::Gantulyst);
    night1Run1GarryCap.setCapshotTime(0.121);
    night1Run1GarryCap.setHealingPhaseTime(12.1);
    night1Run1GarryCap.setResult(CapState::Capture);
    night1Run1GarryCap.setShrineActivationTime(11.1);
    night1Run1GarryCap.setShrineTime(10.2);
    night1Run1GarryCap.setSpawnDelay(0.4);
    night1Run1GarryCap.setWaterShield(2.7);
    night1Run1GarryCap.setSpawned(true);
    night1Run1GarryCap.setValid(true);

    CapInfo night1Run1HarryCap(night1Run1GarryCap);
    night1Run1HarryCap.setEidolon(Eidolon::Hydrolyst);

    night1Run1.setTerralystCapInfo(night1Run1TerryCap);
    night1Run1.setGantulystCapInfo(night1Run1GarryCap);
    night1Run1.setHydrolystCapInfo(night1Run1HarryCap);

    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);
    night1.addRun(night1Run1);

    huntInfo->addNight(night1);
    huntInfo->addNight(night1);

    return huntInfo;

}

}
