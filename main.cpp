#include "TrendInUWO.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TrendInUWO w;
    w.show();
    return a.exec();
}
