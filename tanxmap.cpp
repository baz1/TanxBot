#include "tanxmap.h"

#include <stdio.h>

static TanxMap *_instance = NULL;

TanxMap::TanxMap()
{
    FILE *mapFile = fopen("map.txt", "r");
    if (!mapFile)
    {
        fprintf(stderr, "Error: File \"map.txt\" not found.\n");
        return;
    }
    while (true)
    {
        Border border;
        int tmp;
        fscanf(mapFile, "%d", &tmp);
        if (tmp < 0)
            break;
        border.x1 = tmp;
        fscanf(mapFile, "%d", &tmp);
        border.y1 = tmp;
        fscanf(mapFile, "%d", &tmp);
        border.x2 = tmp;
        fscanf(mapFile, "%d", &tmp);
        border.y2 = tmp;
        borders.append(border);
    }
    fclose(mapFile);
}

TanxMap *TanxMap::instance()
{
    return _instance;
}
