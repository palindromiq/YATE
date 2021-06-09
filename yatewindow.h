#ifndef YATEWINDOW_H
#define YATEWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class YATEWindow; }
QT_END_NAMESPACE

class YATEWindow : public QMainWindow
{
    Q_OBJECT

public:
    YATEWindow(QWidget *parent = nullptr);
    ~YATEWindow();

private:
    Ui::YATEWindow *ui;
};
#endif // YATEWINDOW_H
