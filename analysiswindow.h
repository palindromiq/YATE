#ifndef ANALYSISWINDOW_H
#define ANALYSISWINDOW_H

#include <QMainWindow>

namespace Ui {
class AnalysisWindow;
}
namespace Yate {
class AnalysisViewModel;
class HuntInfo;

class AnalysisWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnalysisWindow(QWidget *parent = nullptr);
    ~AnalysisWindow();

    HuntInfo *hunt() const;
    void setHunt(HuntInfo *newHunt);
    HuntInfo *createDummyHunt();

private slots:
    void on_btnClose_clicked();

private:
    Ui::AnalysisWindow *ui;
    AnalysisViewModel *model_;
    HuntInfo *hunt_;
};
}

#endif // ANALYSISWINDOW_H
