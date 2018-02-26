#include <QApplication>
#include <QtCore>
#include "mainwindow.h"
#include "readusb.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ReadUSB readusb;
    readusb.start();

    MainWindow w;
    w.show();

    return a.exec();
}


