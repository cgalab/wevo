#include "sites.h"
#include "util.h"
#include "log.h"

Site::Site(int weight, int id)
: m_weight{weight}
, m_id{id}
{
}

bool Site::operator<(const Site &other) const {
    if (m_weight != other.weight()) {
        return m_weight < other.weight();
    }
    
    return m_id < other.id();
}

double Site::angle(const Circular_arc_point_2 &arcPnt) const {
    double theta = std::atan2(CGAL::to_double(arcPnt.y()) - CGAL::to_double(center().y()), 
                              CGAL::to_double(arcPnt.x()) - CGAL::to_double(center().x()));
    
    if (theta < 0.) {
        theta += 2 * M_PI;
    }
    
    return theta;
}

PntSite::PntSite(const Point_2 &pnt, int weight, int id)
: Site{weight, id}
, m_pnt{pnt}
{
}

Circle_2 PntSite::growOffAt(const Root_of_2 &sqrdTime) const {
    return Circle_2{m_pnt, CGAL::to_double(sqrdTime) * CGAL::square(weight())};
}

Root_of_2 PntSite::sqrdDist(const Circular_arc_point_2 &arcPnt) const {
    Root_of_2 res = (CGAL::square(arcPnt.x() - m_pnt.x()) 
            + CGAL::square(arcPnt.y() - m_pnt.y())) / CGAL::square(weight());
    return res;
}

#ifdef ENABLE_VIEW

SiteGraphicsItem::SiteGraphicsItem(const SitePtr &site)
: m_site{site}
{
    setAcceptHoverEvents(true);
    setToolTip("id " + QString::number(m_site->id()) + "\nweight " 
            + QString::number(CGAL::to_double(m_site->weight())));
}

QRectF SiteGraphicsItem::boundingRect() const {
    return m_boundingRect;
}

void SiteGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*,
                             QWidget*) {
    if (typeid(*m_site) == typeid(PntSite)) {
        const auto pntSite = std::static_pointer_cast<PntSite>(m_site);
        painter->setPen(Qt::darkGreen);
        painter->setBrush(Qt::darkGreen);
        Util::draw(painter, pntSite->pnt());
    }
    
    const auto scale = std::max(painter->worldTransform().m11(), 
                                painter->worldTransform().m22());
    double x = CGAL::to_double(m_site->center().x()),
            y = CGAL::to_double(m_site->center().y()),
            offset = 10. / scale;
    prepareGeometryChange();
    m_boundingRect = QRectF{x - offset, y - offset, 2. * offset, 2. * offset};
}

void SiteGraphicsItem::modelChanged() {
    update();
}

void SiteGraphicsItem::onToggle(bool bIsVisible) {
    setVisible(bIsVisible);
    update();
}

#include "moc_sites.cpp"

#endif
