#ifndef TANXMAP_H
#define TANXMAP_H

#include <QList>

class TanxMap
{
private:
    struct Border {
        int x1, y1, dX, dY;
    };
public:
    static bool initialize();
    static double getDuration(double x, double y, double dx, double dy);
private:
    static QList<Border> borders;
};

#endif // TANXMAP_H
