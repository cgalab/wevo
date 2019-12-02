#ifndef UTIL_H
#define UTIL_H

#ifdef ENABLE_VIEW

#include <QRectF>
#include <QPainter>
#include "defs.h"

class Util {
public:
    Util() = delete;
    Util(const Util &) = delete;
    
    static Point_2 toPoint_2(const Circular_arc_point_2 &arcPnt);
    
    static void draw(QPainter *painter, const Point_2 &pnt);
    static void draw(QPainter *painter, const Circular_arc_point_2 &arcPnt);
    static void draw(QPainter *painter, const Circle_2 &circ);
    static void draw(QPainter *painter, const Circular_arc_2 &arc);
    static void draw(QPainter *painter, const Line_arc_2 &seg);
    
    static QRectF boundingRect(const Point_2 &pnt);
    static QRectF boundingRect(const Circle_2 &circ);
};

#endif

#endif /* UTIL_H */
