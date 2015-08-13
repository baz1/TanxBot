#include "tanxplayer.h"

#include <math.h>
#include <stdio.h>

TanxPlayer::TanxPlayer(TanxInterface *interface) : QObject(interface), interface(interface), lastRep(NULL_REPULSION)
{
    QObject::connect(interface, SIGNAL(initialized()), this, SLOT(initialized()), Qt::DirectConnection);
    QObject::connect(interface, SIGNAL(gotUpdate()), this, SLOT(gotUpdate()), Qt::DirectConnection);
}

void TanxPlayer::initialized()
{
    const Tank &me = interface->data.tanks.value(interface->data.myID);
    switch (me.team)
    {
    case TEAM_BLUE:
        printf("Team: blue\n");
        break;
    case TEAM_GREEN:
        printf("Team: green\n");
        break;
    case TEAM_RED:
        printf("Team: red\n");
        break;
    case TEAM_YELLOW:
        printf("Team: yellow\n");
        break;
    }
    fflush(stdout);
    interface->setName("AAAA");
}

void TanxPlayer::gotUpdate()
{
    const Tank &me = interface->data.tanks.value(interface->data.myID);
    if (me.dead)
        return;
    double x = me.x + (30 * 0.006) * lastRep.rx, y = me.y + (30 * 0.006) * lastRep.ry;
    /* Border repulsion */
    Repulsion rep = TanxMap::getBordersRepulsion(x, y);
    rep.rx /= 10;
    rep.ry /= 10;
    /* Bullet repulsion */
    QMap<int, Bullet>::iterator bulletIter = interface->data.bullets.begin();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 timestamp = now + 30;
    while (bulletIter != interface->data.bullets.end())
    {
        if (now >= bulletIter.value().expire)
        {
            bulletIter = interface->data.bullets.erase(bulletIter);
            continue;
        }
        Repulsion tmp = TanxMap::getTrajectoryRepulsion(x, y, bulletIter.value(), timestamp);
        if (tmp.isNegligible())
        {
            bulletIter = interface->data.bullets.erase(bulletIter);
            continue;
        }
        rep.rx += tmp.rx;
        rep.ry += tmp.ry;
        ++bulletIter;
    }
    /* Moving management */
    double norm = sqrt(rep.rx * rep.rx + rep.ry * rep.ry);
    if (norm == 0)
    {
        if ((lastRep.rx != 0) || (lastRep.ry != 0))
        {
            interface->move(0., 0.);
            lastRep = NULL_REPULSION;
        }
        return;
    }
    norm = 1. / norm;
    rep.rx *= norm;
    rep.ry *= norm;
    interface->move(rep.rx, rep.ry);
    lastRep = rep;
}
