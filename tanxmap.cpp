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
    if (alpha > bullet.duration)
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
        int len = qMax(qAbs(border.dX), qAbs(border.dY));
        double alpha = (x - border.x1) * border.dX + (y - border.y1) * border.dY;
        alpha /= len;
        double beta = (x - border.x1) * border.dY + (border.y1 - y) * border.dX;
        beta /= len;
        if (beta >= 0)
            continue;
        if (alpha <= 0)
        {
            double rx = x - border.x1;
            double ry = y - border.y1;
            double dist = sqrt(rx * rx + ry * ry);
            if (dist >= 3.)
                continue;
            dist = 1. / (dist * dist * dist);
            result.rx += rx * dist;
            result.ry += ry * dist;
            continue;
        }
        if (alpha > len) // (only horizontal and vertical borders)
        {
            double rx = x - (border.x1 + border.dX);
            double ry = y - (border.y1 + border.dY);
            double dist = sqrt(rx * rx + ry * ry);
            if (dist >= 3.)
                continue;
            dist = 1. / (dist * dist * dist);
            result.rx += rx * dist;
            result.ry += ry * dist;
            continue;
        }
        if ((qAbs(beta) * len) >= 3.)
            continue;
        beta = 1. / (beta * beta);
        result.rx += -border.dY * beta;
        result.ry += border.dX * beta;
    }
    return result;
}
