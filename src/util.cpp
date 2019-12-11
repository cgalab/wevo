#ifdef ENABLE_VIEW

#include <cmath>
#include "util.h"

const double V_SC = 1e-4;

void Util::draw(QPainter *painter, const Point_2 &pnt) {
    double x = CGAL::to_double(pnt.x()) * V_SC,
            y = CGAL::to_double(pnt.y()) * V_SC;
    painter->drawEllipse(QPointF{x, y}, 1., 1.);
}

void Util::draw(QPainter *painter, const Circular_arc_point_2 &arcPnt) {
    double x = CGAL::to_double(arcPnt.x()) * V_SC,
            y = CGAL::to_double(arcPnt.y()) * V_SC;
    painter->drawEllipse(QPointF{x, y}, 1., 1.);
}

void Util::draw(QPainter *painter, const Circle_2 &circ) {
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())) * V_SC,
            x = CGAL::to_double(circ.center().x()) * V_SC,
            y = CGAL::to_double(circ.center().y()) * V_SC;
    painter->drawEllipse(QPointF{x, y}, radius, radius);
}

void Util::draw(QPainter *painter, const Circular_arc_2 &arc) {
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
    
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())) * V_SC,
            x = CGAL::to_double(center.x()) * V_SC,
            y = CGAL::to_double(center.y()) * V_SC;
    const auto rect = QRectF{x - radius, y - radius, radius * 2., radius * 2.};

    if (radius < 3000.) {
        painter->drawArc(rect, static_cast<int> (asource * coeff),
                         static_cast<int> (aspan * coeff));
    } else {
        double x1 = CGAL::to_double(source.x()) * V_SC,
                y1 = CGAL::to_double(source.y()) * V_SC,
                x2 = CGAL::to_double(target.x()) * V_SC,
                y2 = CGAL::to_double(target.y()) * V_SC;
        const auto line = QLineF{QPointF{x1, y1}, QPointF{x2, y2}};
        painter->drawLine(line);
    }
}

void Util::draw(QPainter *painter, const Line_arc_2 &seg) {
    const auto source = seg.source();
    const auto target = seg.target();

    double x1 = CGAL::to_double(source.x()) * V_SC,
            y1 = CGAL::to_double(source.y()) * V_SC,
            x2 = CGAL::to_double(target.x()) * V_SC,
            y2 = CGAL::to_double(target.y()) * V_SC;
    const auto line = QLineF{QPointF{x1, y1}, QPointF{x2, y2}};
    painter->drawLine(line);
}

QRectF Util::boundingRect(const Point_2 &pnt) {
    double x = CGAL::to_double(pnt.x()) * V_SC,
            y = CGAL::to_double(pnt.y()) * V_SC;
    return QRectF{x - 1., y - 1., 2., 2.};
}

QRectF Util::boundingRect(const Circle_2 &circ) {
    double radius = std::sqrt(CGAL::to_double(circ.squared_radius())) * V_SC,
            x = CGAL::to_double(circ.center().x()) * V_SC,
            y = CGAL::to_double(circ.center().y()) * V_SC;
    return QRectF{x - radius - 2., y - radius - 2.,
                  2. * radius + 4., 2. * radius + 4.};
}

#endif
