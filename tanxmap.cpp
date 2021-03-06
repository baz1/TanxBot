#include "tanxmap.h"

#include <stdio.h>
#include <math.h>

QList<TanxMap::Border> TanxMap::borders;
QList<TanxMap::MapZone> TanxMap::mapZones;

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
        fscanf(mapFile, "%d%d%d", &border.y1, &border.dX, &border.dY);
        border.dX -= border.x1;
        border.dY -= border.y1;
        borders.append(border);
    }
    while (true)
    {
        MapZone zone;
        fscanf(mapFile, "%d", &zone.x1);
        if (zone.x1 < 0)
            break;
        fscanf(mapFile, "%d%d%d", &zone.y1, &zone.x2, &zone.y2);
        while (true)
        {
            TargetZone targetZone;
            fscanf(mapFile, "%d", &targetZone.x1);
            if (targetZone.x1 < 0)
                break;
            fscanf(mapFile, "%d%d%d", &targetZone.y1, &targetZone.x2, &targetZone.y2);
            fscanf(mapFile, "%d%d", &targetZone.tx, &targetZone.ty);
            zone.targetZones.append(targetZone);
        }
        fscanf(mapFile, "%d%d", &zone.default_tx, &zone.default_ty);
        mapZones.append(zone);
    }
    fclose(mapFile);
    return true;
}

void TanxMap::initPickables(Pickable *ptr)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    ptr[0] = Pickable(Pickable::Repair, 9.5, 24.5, now);
    ptr[1] = Pickable(Pickable::Repair, 23.5, 9.5, now);
    ptr[2] = Pickable(Pickable::Repair, 38.5, 23.5, now);
    ptr[3] = Pickable(Pickable::Repair, 24.5, 38.5, now);
    ptr[4] = Pickable(Pickable::Damage, 13.5, 15.5, now);
    ptr[5] = Pickable(Pickable::Damage, 32.5, 13.5, now);
    ptr[6] = Pickable(Pickable::Damage, 34.5, 32.5, now);
    ptr[7] = Pickable(Pickable::Damage, 15.5, 34.5, now);
    ptr[8] = Pickable(Pickable::Shield, 24, 24, now);
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
        double dist = result.rx * result.rx + result.ry * result.ry;
        dist = 1. / (dist * dist);
        result.rx *= dist;
        result.ry *= dist;
        return result;
    }
    if (alpha > bullet.duration + BULLET_SEC_DIST)
    {
        result.rx = x - (bullet.x + bullet.dx * bullet.duration);
        result.ry = y - (bullet.y + bullet.dy * bullet.duration);
        double dist = result.rx * result.rx + result.ry * result.ry;
        dist = 1. / (dist * dist);
        result.rx *= dist;
        result.ry *= dist;
        return result;
    }
    double beta = (x - bullet.x) * bullet.dy + (bullet.y - y) * bullet.dx;
    bool neg = (beta > 0);
    beta = 1. / qAbs(beta * beta * beta);
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
        beta = qAbs(beta);
        if ((beta * len) >= WALL_DIST_IGNORE)
            continue;
        beta = 1. / (beta * len * len);
        result.rx += -border.dY * beta;
        result.ry += border.dX * beta;
    }
    return result;
}

Shoot TanxMap::getShoot(double x, double y, double ex, double ey, double dx, double dy)
{
    double t1, t2, d1, d2;
    double exx = ex - x, eyy = ey - y;
    Q_ASSERT(dx * dx + dy * dy < 0.25);
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

Repulsion TanxMap::getUnitAttraction(double x, double y, double tx, double ty)
{
    Repulsion result = getAttraction(x, y, tx, ty);
    double dist = sqrt(result.rx * result.rx + result.ry * result.ry);
    result.rx /= dist;
    result.ry /= dist;
    return result;
}

Repulsion TanxMap::getAttraction(double x, double y, double tx, double ty)
{
    foreach (const MapZone &zone, mapZones)
    {
        if ((x >= zone.x1) && (y >= zone.y1) && (x <= zone.x2) && (y <= zone.y2))
        {
            if ((tx >= zone.x1) && (ty >= zone.y1) && (tx <= zone.x2) && (ty <= zone.y2))
                return Repulsion(tx - x, ty - y);
            foreach (const TargetZone &targetZone, zone.targetZones)
            {
                if ((tx >= targetZone.x1) && (ty >= targetZone.y1) && (tx <= targetZone.x2) && (ty <= targetZone.y2))
                    return Repulsion(targetZone.tx - x, targetZone.ty - y);
            }
            return Repulsion(zone.default_tx - x, zone.default_ty - y);
        }
    }
    return Repulsion(tx - x, ty - y);
}
