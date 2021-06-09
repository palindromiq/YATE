#include "yatewindow.h"
#include "ui_yatewindow.h"

YATEWindow::YATEWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::YATEWindow)
{
    ui->setupUi(this);
}

YATEWindow::~YATEWindow()
{
    delete ui;
}

