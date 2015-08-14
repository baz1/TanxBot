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
    QString name = "AAAA";
    int team = 0;
    while (true)
    {
        TanxInterface tanxThread(NULL, false);
        UserInterface userThread;
        TanxPlayer tanxPlayer(&tanxThread, name, team);
        QEventLoop evtLoop;
        QObject::connect(&tanxThread, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
        QObject::connect(&userThread, SIGNAL(finished()), &tanxThread, SLOT(endConnection()));
        userThread.start();
        evtLoop.exec();
        if (!tanxPlayer.gotWrongTeam())
            break;
        printf("Got wrong team. Retrying in a moment...\n");
        fflush(stdout);
        QThread::sleep(1);
    }
    return 0;
}
