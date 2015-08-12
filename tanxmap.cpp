#include "tanxmap.h"

#include <stdio.h>

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
