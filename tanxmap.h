#ifndef TANXMAP_H
#define TANXMAP_H

#include <QList>
#include <QDateTime>
#include "tanxinterface.h"

#define DELAY_MS_DEFAULT        30
#define REPULSION_NEGLIGIBLE    0.028
#define WALL_DIST_IGNORE        2.

struct Repulsion
{
    double rx, ry;
    bool isNegligible() const
    {
        return (rx * rx + ry * ry < REPULSION_NEGLIGIBLE);
    }
};

const Repulsion NULL_REPULSION = {0., 0.};

class TanxMap
{
private:
    struct Border {
        int x1, y1, dX, dY;
    };
public:
    static bool initialize();
    static double getDuration(double x, double y, double dx, double dy);
    static Repulsion getTrajectoryRepulsion(double x, double y, const Bullet &bullet,
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch() + DELAY_MS_DEFAULT);
    static Repulsion getBordersRepulsion(double x, double y);
private:
    static QList<Border> borders;
};

#endif // TANXMAP_H
