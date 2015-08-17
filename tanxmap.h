#ifndef TANXMAP_H
#define TANXMAP_H

#include <QList>
#include <QDateTime>
#include "tanxinterface.h"

#define DELAY_MS_DEFAULT        30
#define REPULSION_NEGLIGIBLE    0.028
#define WALL_DIST_IGNORE        2.
#define BULLET_SEC_DIST         0.2

struct Repulsion
{
    double rx, ry;
    bool isNegligible() const
    {
        return (rx * rx + ry * ry < REPULSION_NEGLIGIBLE);
    }
};

const Repulsion NULL_REPULSION = {0., 0.};

struct Shoot {
    double angle, dist;
    inline Shoot() {}
    inline Shoot(double angle, double dist) : angle(angle), dist(dist) {}
};

class TanxMap
{
private:
    struct Border {
        int x1, y1, dX, dY;
    };
    struct TargetZone {
        int x1, y1, x2, y2;
        int tx, ty;
    };
    struct MapZone {
        int x1, y1, x2, y2;
        QList<TargetZone> targetZones;
        int default_tx, default_ty;
    };
public:
    static bool initialize();
    static double getDuration(double x, double y, double dx, double dy);
    static Repulsion getTrajectoryRepulsion(double x, double y, const Bullet &bullet,
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch() + DELAY_MS_DEFAULT);
    static Repulsion getBordersRepulsion(double x, double y);
    static Shoot getShoot(double x, double y, double ex, double ey, double dx, double dy);
    static bool isPossible(double x, double y, double ex, double ey, Shoot shoot);
private:
    static QList<Border> borders;
    static QList<MapZone> mapZones;
};

#endif // TANXMAP_H
