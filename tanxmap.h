#ifndef TANXMAP_H
#define TANXMAP_H

#include <QList>

class TanxMap
{
private:
    struct Border {
        double x1, y1, x2, y2;
    };
public:
    TanxMap();
    static TanxMap *instance();
private:
    QList<Border> borders;
};

#endif // TANXMAP_H
