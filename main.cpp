#include <QtCore>

#include "tanxinterface.h"
#include "userinterface.h"
#include "tanxmap.h"

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
