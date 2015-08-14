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
    QString tankName = "guest", followName = "", targetName = "";
    int team = -1;
    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    foreach (const QString &arg, args)
    {
        if (arg.startsWith("name="))
        {
            tankName = arg.mid(5);
            continue;
        }
        if (arg.startsWith("follow="))
        {
            followName = arg.mid(7);
            continue;
        }
        if (arg.startsWith("target="))
        {
            targetName = arg.mid(7);
            continue;
        }
        if (arg.startsWith("team="))
        {
            QString teamName = arg.mid(5).toLower();
            if (teamName == "blue")
            {
                team = TEAM_BLUE;
                continue;
            }
            if (teamName == "red")
            {
                team = TEAM_RED;
                continue;
            }
            if (teamName == "green")
            {
                team = TEAM_GREEN;
                continue;
            }
            if (teamName == "yellow")
            {
                team = TEAM_YELLOW;
                continue;
            }
            fprintf(stderr, "Error: Unrecognized team \"%s\".\n", qPrintable(teamName));
            return 1;
        }
        fprintf(stderr, "Error: Unrecognized option \"%s\".\n", qPrintable(arg));
        return 1;
    }
    while (true)
    {
        TanxInterface tanxThread(NULL, false);
        UserInterface userThread;
        TanxPlayer tanxPlayer(&tanxThread, tankName, followName, targetName, team);
        QEventLoop evtLoop;
        QObject::connect(&tanxThread, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
        QObject::connect(&userThread, SIGNAL(finished()), &tanxThread, SLOT(endConnection()));
        userThread.start();
        evtLoop.exec();
        if (!tanxPlayer.gotWrongTeam())
            break;
        userThread.abort();
        userThread.wait(1000);
        printf("Got wrong team. Retrying in a moment...\n");
        fflush(stdout);
        QThread::sleep(2);
    }
    return 0;
}
