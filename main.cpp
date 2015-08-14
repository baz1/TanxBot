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
            if ((tankName.length() < 4) || (tankName.length() > 8))
            {
                fprintf(stderr, "Error: Name \"%s\" is either too short (<4) or too long (>8).\n", qPrintable(tankName));
                return 1;
            }
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
    TanxInterface *tanxInterface;
    QEventLoop evtLoop;
    {
        QList<TanxInterface*> interfaces;
        TanxPlayer* lastPlayer;
        while (true)
        {
            interfaces.append(new TanxInterface(NULL, false));
            lastPlayer = new TanxPlayer(interfaces.last(), tankName, followName, targetName, team);
            QObject::connect(lastPlayer, SIGNAL(enfOfInitialization()), &evtLoop, SLOT(quit()));
            evtLoop.exec();
            lastPlayer->disconnect();
            if (!lastPlayer->gotWrongTeam())
                break;
        }
        tanxInterface = interfaces.last();
        for (int i = interfaces.length() - 2; i >= 0; --i)
            delete interfaces.at(i);
        interfaces.clear();
    }
    if (!tanxInterface->isConnected())
    {
        fprintf(stderr, "Error: Got disconnected.\n");
        delete tanxInterface;
        return 1;
    }
    UserInterface userInterface;
    QObject::connect(tanxInterface, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
    QObject::connect(&userInterface, SIGNAL(finished()), tanxInterface, SLOT(endConnection()));
    userInterface.start();
    evtLoop.exec();
    delete tanxInterface;
    return 0;
}
