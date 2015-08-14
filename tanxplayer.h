#ifndef TANXPLAYER_H
#define TANXPLAYER_H

#include "tanxinterface.h"
#include "tanxmap.h"

#define DELAY_MS                25
#define SHOOT_DELAY_MS          25
#define IGNORE_DISTANCE         15.

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
    int targetTank;
};

#endif // TANXPLAYER_H
