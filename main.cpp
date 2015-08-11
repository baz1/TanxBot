#include <QtCore>

#include "tanxinterface.h"
#include "userinterface.h"

// Team 0: Blue
// Team 1: Red
// Team 2: Green
// Team 3: Yellow

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TanxInterface tanxThread;
    UserInterface userThread;
    QEventLoop evtLoop;
    QObject::connect(&tanxThread, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
    QObject::connect(&userThread, SIGNAL(finished()), &tanxThread, SLOT(endConnection()));
    userThread.start();
    evtLoop.exec();
    return 0;
}
