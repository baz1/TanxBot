#include "tanxmap.h"

#include <stdio.h>
#include <math.h>

QList<TanxMap::Border> TanxMap::borders;

bool TanxMap::initialize()
{
    FILE *mapFile = fopen("map.txt", "r");
    if (!mapFile)
    {
        fprintf(stderr, "Error: File \"map.txt\" not found.\n");
        return false;
    }
    while (true)
    {
        Border border;
        fscanf(mapFile, "%d", &border.x1);
        if (border.x1 < 0)
            break;
        fscanf(mapFile, "%d", &border.y1);
        fscanf(mapFile, "%d", &border.dX);
        border.dX -= border.x1;
        fscanf(mapFile, "%d", &border.dY);
        border.dY -= border.y1;
        borders.append(border);
    }
    fclose(mapFile);
    return true;
}

double TanxMap::getDuration(double x, double y, double dx, double dy)
{
    double duration = 1000, det, d1, d2, alpha, beta;
    foreach (const Border &border, borders)
    {
        det = dx * border.dY - dy * border.dX;
        if (det <= 0)
            continue;
        det = 1. / det;
        d1 = border.x1 - x;
        d2 = y - border.y1;
        beta = det * (d1 * dy + d2 * dx);
        if ((beta < 0) || (beta > 1))
            continue;
        alpha = det * (d1 * border.dY + d2 * border.dX);
        if (alpha < 0)
            continue;
        if (duration > alpha)
            duration = alpha;
    }
    return duration;
}

Repulsion TanxMap::getTrajectoryRepulsion(double x, double y, const Bullet &bullet, qint64 timestamp)
{
    Repulsion result = NULL_REPULSION;
    double spent = bullet.duration * ((double) (timestamp - bullet.launched)) / ((double) (bullet.expire - bullet.launched));
    spent -= BULLET_SEC_DIST;
    if (spent < 0)
        spent = 0;
    double alpha = (x - bullet.x) * bullet.dx + (y - bullet.y) * bullet.dy;
    if (alpha <= spent)
    {
        result.rx = x - (bullet.x + bullet.dx * spent);
        result.ry = y - (bullet.y + bullet.dy * spent);
        double dist = sqrt(result.rx * result.rx + result.ry * result.ry);
        dist = 1. / (dist * dist * dist);
        result.rx *= dist;
        result.ry *= dist;
        return result;
    }
    if (alpha > bullet.duration + BULLET_SEC_DIST)
    {
        result.rx = x - (bullet.x + bullet.dx * bullet.duration);
        result.ry = y - (bullet.y + bullet.dy * bullet.duration);
        double dist = sqrt(result.rx * result.rx + result.ry * result.ry);
        dist = 1. / (dist * dist * dist);
        result.rx *= dist;
        result.ry *= dist;
        return result;
    }
    double beta = (x - bullet.x) * bullet.dy + (bullet.y - y) * bullet.dx;
    bool neg = (beta > 0);
    beta = 1. / (beta * beta);
    if (neg)
    {
        result.rx = bullet.dy * beta;
        result.ry = -bullet.dx * beta;
    } else {
        result.rx = -bullet.dy * beta;
        result.ry = bullet.dx * beta;
    }
    return result;
}

Repulsion TanxMap::getBordersRepulsion(double x, double y)
{
    Repulsion result = NULL_REPULSION;
    foreach (const TanxMap::Border &border, borders)
    {
        int len = qMax(qAbs(border.dX), qAbs(border.dY)); // (only horizontal and vertical borders)
        double alpha = (x - border.x1) * border.dX + (y - border.y1) * border.dY;
        alpha /= len * len;
        double beta = (x - border.x1) * border.dY + (border.y1 - y) * border.dX;
        beta /= len * len;
        if (beta >= 0)
            continue;
        if (alpha <= 0)
        {
            double rx = x - border.x1;
            double ry = y - border.y1;
            double dist = sqrt(rx * rx + ry * ry);
            if (dist >= WALL_DIST_IGNORE)
                continue;
            dist = 1. / (dist * dist * dist);
            result.rx += rx * dist;
            result.ry += ry * dist;
            continue;
        }
        if (alpha > 1)
        {
            double rx = x - (border.x1 + border.dX);
            double ry = y - (border.y1 + border.dY);
            double dist = sqrt(rx * rx + ry * ry);
            if (dist >= WALL_DIST_IGNORE)
                continue;
            dist = 1. / (dist * dist * dist);
            result.rx += rx * dist;
            result.ry += ry * dist;
            continue;
        }
        if ((qAbs(beta) * len) >= WALL_DIST_IGNORE)
            continue;
        beta = 1. / (beta * beta);
        result.rx += -border.dY * beta;
        result.ry += border.dX * beta;
    }
    return result;
}

Shoot TanxMap::getShoot(double x, double y, double ex, double ey, double dx, double dy)
{
    double t1, t2, d1, d2;
    double exx = ex - x, eyy = ey - y;
    double a = eyy * dx - exx * (1. + dy), hb = -eyy, c = exx * (1 - dy) + eyy * dx;
    if (a == 0)
    {
        t1 = 3.141592653589793238463;
        Q_ASSERT(hb != 0);
        t2 = 2 * atan(-c / (2 * hb));
    } else {
        double delta = hb * hb - a * c;
        Q_ASSERT(delta > 0);
        delta = sqrt(delta);
        t1 = 2 * atan((-hb + delta) / a);
        t2 = 2 * atan((-hb - delta) / a);
    }
    if (qAbs(exx) > qAbs(eyy))
    {
        d1 = exx / (sin(t1) - dx);
        d2 = exx / (sin(t2) - dx);
    } else {
        d1 = eyy / (cos(t1) - dy);
        d2 = eyy / (cos(t2) - dy);
    }
    if (d1 < 0)
    {
        Q_ASSERT(d2 > 0);
        return Shoot(t2, d2);
    }
    if (d2 < 0)
    {
        Q_ASSERT(d1 > 0);
        return Shoot(t1, d1);
    }
    if (d1 < d2)
    {
        return Shoot(t1, d1);
    } else {
        return Shoot(t2, d2);
    }
}

bool TanxMap::isPossible(double x, double y, double ex, double ey, Shoot shoot)
{
    double dx = sin(shoot.angle), dy = cos(shoot.angle);
    if (getDuration(x, y, dx, dy) <= shoot.dist)
        return false;
    dx = (x + shoot.dist * dx) - ex;
    dy = (y + shoot.dist * dy) - ey;
    double dist = sqrt(dx * dx + dy * dy);
    dx /= dist;
    dy /= dist;
    return (getDuration(ex, ey, dx, dy) > dist);
}
