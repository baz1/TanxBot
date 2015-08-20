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
    bool startOff = false;
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
        if (arg == "off")
        {
            startOff = true;
            continue;
        }
        if (arg.startsWith("way="))
        {
            QStringList coords = arg.mid(4).split(QChar(','));
            if (coords.length() != 4)
            {
                fprintf(stderr, "Error: Wrong number of coordinates.\n");
                return 1;
            }
            Repulsion attr = TanxMap::getUnitAttraction(coords.at(0).toDouble(), coords.at(1).toDouble(),
                                                        coords.at(2).toDouble(), coords.at(3).toDouble());
            printf("%lf %lf\n", attr.rx, attr.ry);
            return 0;
        }
        fprintf(stderr, "Error: Unrecognized option \"%s\".\n", qPrintable(arg));
        return 1;
    }
    TanxInterface *tanxInterface;
    TanxPlayer* lastPlayer;
    QEventLoop evtLoop;
    {
        QList<TanxInterface*> interfaces;
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
    if (startOff)
        lastPlayer->setActivated(false);
    UserInterface userInterface;
    QObject::connect(&userInterface, SIGNAL(setActivated(bool)), lastPlayer, SLOT(setActivated(bool)));
    QObject::connect(&userInterface, SIGNAL(setMyName(QString)), lastPlayer, SLOT(setMyName(QString)));
    QObject::connect(&userInterface, SIGNAL(setFollowName(QString)), lastPlayer, SLOT(setFollowName(QString)));
    QObject::connect(&userInterface, SIGNAL(setTargetName(QString)), lastPlayer, SLOT(setTargetName(QString)));
    QObject::connect(tanxInterface, SIGNAL(disconnected()), &evtLoop, SLOT(quit()));
    QObject::connect(&userInterface, SIGNAL(finished()), tanxInterface, SLOT(endConnection()));
    userInterface.start();
    evtLoop.exec();
    delete tanxInterface;
    return 0;
}
