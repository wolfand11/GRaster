#include "graster.h"
#include <QApplication>
#include "glog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GLOG_INFO_CHECK(10<5, "10>5", "true");
    GRaster w;
    w.show();

    return a.exec();
}
