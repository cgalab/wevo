#ifdef ENABLE_VIEW

#include <cmath>
#include "util.h"

void Util::draw(QPainter *painter, const Point_2 &pnt) {
    const auto scale = std::max(painter->worldTransform().m11(), 
                                painter->worldTransform().m22());
    double x = CGAL::to_double(pnt.x()),
            y = CGAL::to_double(pnt.y());
    painter->drawEllipse(QPointF{x, y}, 5. / scale, 5. / scale);
}

void Util::draw(QPainter *painter, const Circular_arc_point_2 &arcPnt, double size) {
    const auto scale = std::max(painter->worldTransform().m11(), 
                                painter->worldTransform().m22());
    double x = CGAL::to_double(arcPnt.x()),
            y = CGAL::to_double(arcPnt.y());
    painter->drawEllipse(QPointF{x, y}, size / scale, size / scale);
}

void Util::draw(QPainter *painter, const Circle_2 &circ) {
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())),
            x = CGAL::to_double(circ.center().x()),
            y = CGAL::to_double(circ.center().y());
    painter->drawEllipse(QPointF{x, y}, radius, radius);
}

void Util::draw(QPainter *painter, const Circular_arc_2 &arc) {
    const auto scale = std::max(painter->worldTransform().m11(), 
                                painter->worldTransform().m22());
    const auto circ = arc.supporting_circle();
    const auto center = circ.center();
    const auto source = arc.source();
    const auto target = arc.target();

    double asource = std::atan2(-CGAL::to_double(source.y() - center.y()),
                                CGAL::to_double(source.x() - center.x()));
    double atarget = std::atan2(-CGAL::to_double(target.y() - center.y()),
                                CGAL::to_double(target.x() - center.x()));

    std::swap(asource, atarget);
    double aspan = atarget - asource;

    if(aspan < 0.) {
      aspan += 2 * CGAL_PI;
    }

    double coeff = 180. * 16. / CGAL_PI;
    
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())),
            x = CGAL::to_double(center.x()),
            y = CGAL::to_double(center.y());
    const auto rect = QRectF{x - radius, y - radius, radius * 2., radius * 2.};

    if (radius < 5000. / scale) {
        painter->drawArc(rect, static_cast<int> (asource * coeff),
                         static_cast<int> (aspan * coeff));
    } else {
        double x1 = CGAL::to_double(source.x()),
                y1 = CGAL::to_double(source.y()),
                x2 = CGAL::to_double(target.x()),
                y2 = CGAL::to_double(target.y());
        const auto line = QLineF{QPointF{x1, y1}, QPointF{x2, y2}};
        painter->drawLine(line);
    }
}

void Util::draw(QPainter *painter, const Line_arc_2 &seg) {
    const auto source = seg.source();
    const auto target = seg.target();

    double x1 = CGAL::to_double(source.x()),
            y1 = CGAL::to_double(source.y()),
            x2 = CGAL::to_double(target.x()),
            y2 = CGAL::to_double(target.y());
    const auto line = QLineF{QPointF{x1, y1}, QPointF{x2, y2}};
    painter->drawLine(line);
}

QRectF Util::boundingRect(const Circle_2 &circ, double offset) {
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())),
            x = CGAL::to_double(circ.center().x()),
            y = CGAL::to_double(circ.center().y());
    return QRectF{x - radius - offset, y - radius - offset,
                  2. * (radius + offset), 2. * (radius + offset)};
}

#endif
