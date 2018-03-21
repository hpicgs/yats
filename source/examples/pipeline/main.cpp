#include <QApplication>
#include <QSurfaceFormat>

#include "window.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    window prog;
    prog.exec();

    return 0;
}
