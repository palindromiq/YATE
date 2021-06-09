#include "yatewindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YATEWindow w;
    w.show();
    return a.exec();
}
