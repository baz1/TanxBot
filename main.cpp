#include <QtCore>

#include "tanxinterface.h"
#include "userinterface.h"
#include "tanxmap.h"
#include "tanxplayer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if (!TanxMap::initialize())
        return 1;
    TanxInterface tanxThread(NULL, false);
    UserInterface userThread;
    TanxPlayer tanxPlayer(&tanxThread);
    QEventLoop evtLoop;
    QObject::connect(&tanxThread, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
    QObject::connect(&userThread, SIGNAL(finished()), &tanxThread, SLOT(endConnection()));
    userThread.start();
    evtLoop.exec();
    return 0;
}
