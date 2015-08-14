#ifndef TANXPLAYER_H
#define TANXPLAYER_H

#include "tanxinterface.h"
#include "tanxmap.h"

#define DELAY_MS                25
#define SHOOT_DELAY_MS          25
#define TANK_IGNORE_DISTANCE    15.
#define PICKABLE_IGN_DISTANCE   15.
#define LOW_LIFE                3
#define ANTICIPATE_MOVE         0.5
#define FOLLOW_DISTANCE         3.
#define REP_THRESHOLD           0.18

class TanxPlayer : public QObject
{
    Q_OBJECT
public:
    explicit TanxPlayer(TanxInterface *interface, QString myName, QString followName, QString targetName, int needTeam);
    inline bool gotWrongTeam() const;
public slots:
    void initialized();
    void gotUpdate();
    void newTank(const Tank &tank);
    void delTank(int id);
private:
    void playUpdate();
private:
    TanxInterface *interface;
    Repulsion lastRep;
    QString myName, followName, targetName;
    int followTank, targetTank, needTeam;
    bool wrongTeam;
};

inline bool TanxPlayer::gotWrongTeam() const
{
    return wrongTeam;
}

#endif // TANXPLAYER_H
