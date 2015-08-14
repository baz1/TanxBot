#include "tanxplayer.h"

#include <math.h>
#include <stdio.h>

TanxPlayer::TanxPlayer(TanxInterface *interface, QString myName, QString followName, QString targetName, int needTeam)
    : QObject(interface), interface(interface), lastRep(NULL_REPULSION),
      myName(myName), followName(followName.toLower()), targetName(targetName.toLower()),
      followTank(-1), targetTank(-1), needTeam(needTeam), wrongTeam(false)
{
    QObject::connect(interface, SIGNAL(initialized()), this, SLOT(initialized()), Qt::DirectConnection);
    QObject::connect(interface, SIGNAL(gotUpdate()), this, SLOT(gotUpdate()), Qt::DirectConnection);
    QObject::connect(interface, SIGNAL(newTank(Tank)), this, SLOT(newTank(Tank)), Qt::DirectConnection);
    QObject::connect(interface, SIGNAL(delTank(int)), this, SLOT(delTank(int)), Qt::DirectConnection);
}

void TanxPlayer::initialized()
{
    const Tank &me = interface->data.tanks.value(interface->data.myID);
    if (needTeam >= 0)
    {
        if (me.team != needTeam)
        {
            wrongTeam = true;
            interface->endConnection();
            return;
        }
        printf("Got the right team!\n");
        fflush(stdout);
    } else {
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
    }
    interface->setName(myName);
}

void TanxPlayer::gotUpdate()
{
    playUpdate();
}

void TanxPlayer::newTank(const Tank &tank)
{
    QString tankName = interface->data.users.value(tank.owner).toLower();
    if ((followTank < 0) && (tankName == followName))
        followTank = tank.id;
    if (tank.team == interface->data.myTeam)
        return;
    if ((targetTank < 0) && (tankName == targetName))
        targetTank = tank.id;
}

void TanxPlayer::delTank(int id)
{
    if (id == followTank)
        followTank = -1;
    if (id == targetTank)
        targetTank = -1;
}

template <typename T> inline T sq(const T &a) { return a * a; }

void TanxPlayer::playUpdate()
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
        printf("a\n");
        const Tank &toFollow = interface->data.tanks.value(followTank);
        double dx = toFollow.x + toFollow.dx * (ANTICIPATE_MOVE * BULLET_SPEED / TANK_SPEED) - x;
        double dy = toFollow.y + toFollow.dy * (ANTICIPATE_MOVE * BULLET_SPEED / TANK_SPEED) - y;
        double val = sqrt(dx * dx + dy * dy);
        val = 0.2 * (val > 3. + FOLLOW_DISTANCE ? 0.6 : val - FOLLOW_DISTANCE) / val;
        rep.rx += dx * val;
        rep.ry += dy * val;
    }
    /* Discard small moves */
    printf("rep: (%lf,%lf)\n", rep.rx, rep.ry);
    double norm = sqrt(rep.rx * rep.rx + rep.ry * rep.ry);
    if ((lastRep.rx == 0) && (lastRep.ry == 0))
    {
        if (norm > REP_NEGLIGIBLE)
        {
            norm = 1. / norm;
            rep.rx *= norm;
            rep.ry *= norm;
        } else {
            rep.rx = 0;
            rep.ry = 0;
        }
    } else {
        if (norm - (rep.rx * lastRep.rx + rep.ry * lastRep.ry) < REP_THRESHOLD)
        {
            rep = lastRep;
        } else {
            if (norm > REP_NEGLIGIBLE)
            {
                norm = 1. / norm;
                rep.rx *= norm;
                rep.ry *= norm;
            } else {
                rep.rx = 0;
                rep.ry = 0;
            }
        }
    }
    /* Shoot ennemies */
    x += rep.rx * (SHOOT_DELAY_MS * TANK_SPEED / 1000.);
    y += rep.ry * (SHOOT_DELAY_MS * TANK_SPEED / 1000.);
    Shoot shoot, bestShoot;
    bestShoot.dist = 100;
    bool bestIsLowLife = false;
    foreach (const Tank &tank, interface->data.tanks)
    {
        if (tank.dead || (tank.team == interface->data.myTeam))
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
                if (TanxMap::isPossible(x, y, tank.x, tank.y, shoot))
                {
                    bestShoot = shoot;
                    bestIsLowLife = (tank.hp <= LOW_LIFE);
                }
            }
        }
    }
    /* Moving and shooting interface management */
    if ((rep.rx == lastRep.rx) && (rep.ry == lastRep.ry))
    {
        interface->setTarget(bestShoot.angle, bestShoot.dist != 100);
    } else {
        interface->targettedMove(bestShoot.angle, rep.rx, rep.ry, bestShoot.dist != 100);
    }
    lastRep = rep;
    printf("lastrep: (%lf,%lf)\n", rep.rx, rep.ry);
}
