#ifndef TANXMAP_H
#define TANXMAP_H

#include <QList>
#include <QDateTime>
#include "tanxinterface.h"

struct Repulsion
{
    double rx, ry;
};

Repulsion operator+(Repulsion a, Repulsion b);

class TanxMap
{
private:
    struct Border {
        int x1, y1, dX, dY;
    };
public:
    static bool initialize();
    static double getDuration(double x, double y, double dx, double dy);
    static Repulsion getTrajectoryRepulsion(double x, double y, const Bullet &bullet, qint64 timestamp = QDateTime::currentMSecsSinceEpoch() + 30);
    static Repulsion getBordersRepulsion(double x, double y);
private:
    static QList<Border> borders;
};

#endif // TANXMAP_H
