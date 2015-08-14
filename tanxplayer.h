#ifndef TANXPLAYER_H
#define TANXPLAYER_H

#include "tanxinterface.h"
#include "tanxmap.h"

#define DELAY_MS                25
#define SHOOT_DELAY_MS          25
#define TANK_IGNORE_DISTANCE    15.
#define PICKABLE_IGN_DISTANCE   10.
#define LOW_LIFE                3
#define ANTICIPATE_MOVE         0.5

class TanxPlayer : public QObject
{
    Q_OBJECT
public:
    explicit TanxPlayer(TanxInterface *interface);
public slots:
    void initialized();
    void gotUpdate();
private:
    TanxInterface *interface;
    Repulsion lastRep;
    int followTank, targetTank;
};

#endif // TANXPLAYER_H
