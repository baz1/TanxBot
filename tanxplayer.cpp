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

template <typename T> inline T sq(const T &a) { return a * a; }

void TanxPlayer::gotUpdate()
{
    const Tank &me = interface->data.tanks.value(interface->data.myID);
    if (me.dead)
        return;
    double x = me.x + (DELAY_MS * TANK_SPEED / 1000.) * lastRep.rx, y = me.y + (DELAY_MS * TANK_SPEED / 1000.) * lastRep.ry;
    /* Border repulsion */
    Repulsion rep = TanxMap::getBordersRepulsion(x, y);
    rep.rx /= 10;
    rep.ry /= 10;
    /* Bullet repulsion */
    QMap<int, Bullet>::iterator bulletIter = interface->data.bullets.begin();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 timestamp = now + DELAY_MS;
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
    /* Pickables attraction */
    foreach (const Pickable &pickable, interface->data.pickables)
    {
        double dx = pickable.x - x, dy = pickable.y - y;
        double val = dx * dx + dy * dy;
        if (val > PICKABLE_IGN_DISTANCE * PICKABLE_IGN_DISTANCE)
            continue;
        if (TanxMap::getDuration(x, y, dx, dy) < sqrt(val))
            continue;
        val = 1. / val;
        switch (pickable.t)
        {
        case Pickable::Repair:
            val *= (0.25 * (10 - me.hp)) / me.hp;
            break;
        case Pickable::Damage:
            val *= 0.5;
            break;
        case Pickable::Shield:
            val *= 0.07 * (10 - me.sp);
            break;
        }
        rep.rx += dx * val;
        rep.ry += dy * val;
    }
    /* Following attraction */
    if (interface->data.tanks.contains(followTank))
    {
        const Tank &toFollow = interface->data.tanks.value(followTank);
        double dx = toFollow.x + toFollow.dx * (ANTICIPATE_MOVE * BULLET_SPEED / TANK_SPEED) - x;
        double dy = toFollow.y + toFollow.dy * (ANTICIPATE_MOVE * BULLET_SPEED / TANK_SPEED) - y;
        double val = 0.7 / sqrt(dx * dx + dy * dy);
        dx *= val;
        dy *= val;
    }
    /* Shoot ennemies */
    x += rep.rx * (SHOOT_DELAY_MS * TANK_SPEED / 1000.);
    y += rep.ry * (SHOOT_DELAY_MS * TANK_SPEED / 1000.);
    Shoot shoot, bestShoot;
    bestShoot.dist = 100;
    bool bestIsLowLife = false;
    foreach (const Tank &tank, interface->data.tanks)
    {
        if (tank.team == interface->data.myTeam)
            continue;
        if (sq(x - tank.x) + sq(y - tank.y) > TANK_IGNORE_DISTANCE * TANK_IGNORE_DISTANCE)
            continue;
        shoot = TanxMap::getShoot(x, y, tank.x, tank.y, tank.dx, tank.dy);
        if (tank.id == targetTank)
        {
            if (TanxMap::isPossible(x, y, tank.x, tank.y, shoot))
            {
                bestShoot = shoot;
                break;
            }
        } else {
            if ((shoot.dist < bestShoot.dist) && ((!bestIsLowLife) || (tank.hp <= LOW_LIFE)))
            {
                bestShoot = shoot;
                bestIsLowLife = (tank.hp <= LOW_LIFE);
            }
        }
    }
    /* Moving and shooting interface management */
    double norm = sqrt(rep.rx * rep.rx + rep.ry * rep.ry);
    if (norm == 0)
    {
        if ((lastRep.rx != 0) || (lastRep.ry != 0))
        {
            interface->targettedMove(bestShoot.angle, 0., 0., bestShoot.dist != 100);
            lastRep = NULL_REPULSION;
        } else {
            interface->setTarget(bestShoot.angle, bestShoot.dist != 100);
        }
    } else {
        norm = 1. / norm;
        rep.rx *= norm;
        rep.ry *= norm;
        interface->targettedMove(bestShoot.angle, rep.rx, rep.ry, bestShoot.dist != 100);
        lastRep = rep;
    }
}
