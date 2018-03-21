#include "Builder.h"

#include <QApplication>

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    
    Builder builder;
    builder.process(app);

    return 0;
}
