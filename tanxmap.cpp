#include "tanxmap.h"

#include <stdio.h>

static TanxMap *_instance = new TanxMap();

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
        fscanf(mapFile, "%d", &border.x1);
        if (border.x1 < 0)
            break;
        fscanf(mapFile, "%d", &border.y1);
        fscanf(mapFile, "%d", &border.x2);
        fscanf(mapFile, "%d", &border.y2);
        borders.append(border);
    }
    fclose(mapFile);
}

TanxMap *TanxMap::instance()
{
    return _instance;
}
