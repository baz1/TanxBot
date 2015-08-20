#ifndef TANXPLAYER_H
#define TANXPLAYER_H

#include "tanxinterface.h"
#include "tanxmap.h"

#define DELAY_MS                25
#define SHOOT_DELAY_MS          25
#define TANK_IGNORE_DISTANCE    15.
#define PICKABLE_IGN_DISTANCE   20.
#define LOW_LIFE                3
#define ANTICIPATE_MOVE         0.5
#define FOLLOW_DISTANCE         3.
#define REP_THRESHOLD           0.18
#define REP_NEGLIGIBLE          0.05
#define DIST_CROW_FLIES_RATIO   1.4

class TanxPlayer : public QObject
{
    Q_OBJECT
public:
    explicit TanxPlayer(TanxInterface *interface, QString myName, QString followName, QString targetName, int needTeam);
    inline bool gotWrongTeam() const;
signals:
    void enfOfInitialization();
public slots:
    void initialized();
    void gotUpdate();
    void newTank(const Tank &tank);
    void delTank(int id);
    void newUserName(QString id, QString name);
    void setActivated(bool enabled);
private:
    void playUpdate();
private:
    TanxInterface *interface;
    Repulsion lastRep;
    QString myName, followName, targetName;
    int followTank, targetTank, needTeam;
    bool wrongTeam, active;
};

inline bool TanxPlayer::gotWrongTeam() const
{
    return wrongTeam;
}

#endif // TANXPLAYER_H
