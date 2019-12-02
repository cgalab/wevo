#include <iostream>
#include <cmath>
#include "util.h"
#include "bisector.h"

std::pair<int, int> Bisec::id() const {
    return std::minmax({site1()->id(), site2()->id()});
}

BisecPntPnt::BisecPntPnt() {
}

BisecPntPnt::BisecPntPnt(const PntSitePtr &site1, const PntSitePtr &site2)
: m_site1{site1}
, m_site2{site2}
{
    //CGAL_assertion(m_site1->weight() != m_site2->weight());
    
    //if (m_site1->weight() < m_site2->weight()) {
    if (*m_site1 < *m_site2) {
        std::swap(m_site1, m_site2);
    }
    
    m_centerLine = Line_2{m_site2->pnt(), m_site1->pnt()};
    compTrajs();
}

Line_2 BisecPntPnt::toLine_2() const {
    const auto p = m_site1->pnt(), q = m_site2->pnt();
    const auto midPnt = Point_2{(p.x() + q.x()) / 2, (p.y() + q.y()) / 2};
    return m_centerLine.perpendicular(midPnt);
}

Circle_2 BisecPntPnt::toCircle_2() const {
    const auto sqrdWgt1 = CGAL::square(m_site1->weight()),
            sqrdWgt2 = CGAL::square(m_site2->weight());
    
    const auto pnt1 = m_site1->pnt(), pnt2 = m_site2->pnt();
    const auto sqrdDist = CGAL::squared_distance(pnt1, pnt2);

    const auto sqrdRadius = (sqrdWgt1 * sqrdWgt2 * sqrdDist)
            / CGAL::square(sqrdWgt2 - sqrdWgt1);

    const auto denom = sqrdWgt1 - sqrdWgt2;
    const auto centerX = (sqrdWgt1 * pnt2.x() - sqrdWgt2 * pnt1.x()) / denom,
            centerY = (sqrdWgt1 * pnt2.y() - sqrdWgt2 * pnt1.y()) / denom;

    return Circle_2{Point_2{centerX, centerY}, sqrdRadius};
}

const TrajPtr & BisecPntPnt::findTraj(const Circular_arc_point_2 &arcPnt) const {
    const auto pnt = Point_2{CGAL::to_double(arcPnt.x()),
                             CGAL::to_double(arcPnt.y())};
                             
    CGAL::Oriented_side side = m_centerLine.oriented_side(pnt);
    CGAL_assertion(side != CGAL::ON_ORIENTED_BOUNDARY);
    bool bIsPositive = (side == CGAL::ON_POSITIVE_SIDE);
    return m_trajs.at(bIsPositive ? 0 : 1);
}

void BisecPntPnt::compTrajs() {
    if (m_site1->weight() == m_site2->weight()) {
        const auto line = toLine_2();
        if(!line.is_vertical()) {
            bool bIsYGreater = !(m_site1->pnt().y() < m_site2->pnt().y());
            const auto sign = FT{bIsYGreater ? 1 : -1},
                    x1 = FT{sign * -1e12}, x2 = FT{sign * 1e12},
                    y1 = line.y_at_x(x1), y2 = line.y_at_x(x2);
            const auto p = m_site1->pnt(), q = m_site2->pnt();
            const auto midPnt = Point_2{(p.x() + q.x()) / 2, (p.y() + q.y()) / 2},
                    endPnt1 = Point_2{x1, y1}, endPnt2 = Point_2{x2, y2};
            auto t1 = m_site1->sqrdDist(midPnt);
            const auto seg1 = Line_arc_2{midPnt, endPnt1}, seg2 = Line_arc_2{midPnt, endPnt2};

            m_coll = std::make_shared<TransitPnt>(Circular_arc_point_2{midPnt}, t1, id(), id(), PntType::Coll);
            m_dom = std::make_shared<TransitPnt>(Circular_arc_point_2{Point_2{FT{1e12}, FT{1e12}}}, Root_of_2{1e12}, id(), id(), PntType::Dom);

            const auto trajSec1 = std::make_shared<TrajSecPntPnt>(m_coll, m_dom, m_site1, m_site2, false, seg1),
                    trajSec2 = std::make_shared<TrajSecPntPnt>(m_coll, m_dom, m_site1, m_site2, true, seg2);

            const auto traj1 = std::make_shared<Traj>(m_site1, m_site2, true),
                    traj2 = std::make_shared<Traj>(m_site1, m_site2, false);

            traj1->insSec(trajSec1);
            traj2->insSec(trajSec2);

            m_trajs.push_back(traj1);
            m_trajs.push_back(traj2);
        } else {
            CGAL_assertion(false);
        }
    } else {
        const auto circ = toCircle_2();
        using IsectResult
                = CGAL::CK2_Intersection_traits<CK, Circle_2, Line_2>::type;
        using Isect = std::pair<Circular_arc_point_2, unsigned int>;
        std::vector<IsectResult> results;
        CGAL::intersection(circ, m_centerLine, std::back_inserter(results));

        CGAL_assertion(results.size() == 2);
        if (results.size() != 2) {
            return;
        }

        auto pnt1 = std::get<0>(boost::get<Isect>(results.at(0))),
                pnt2 = std::get<0>(boost::get<Isect>(results.at(1)));
        auto t1 = m_site1->sqrdDist(pnt1), t2 = m_site1->sqrdDist(pnt2);

        if (t1 > t2) {
            std::swap(pnt1, pnt2);
            std::swap(t1, t2);
        }

        m_coll = std::make_shared<TransitPnt>(pnt1, t1, id(), id(), PntType::Coll);
        m_dom = std::make_shared<TransitPnt>(pnt2, t2, id(), id(), PntType::Dom);

        const auto arc1 = Circular_arc_2{circ, pnt1, pnt2},
                    arc2 = Circular_arc_2{circ, pnt2, pnt1};

        const auto trajSec1 = std::make_shared<TrajSecPntPnt>(m_coll, m_dom, m_site1,
                                                              m_site2, false, arc1),
                trajSec2 = std::make_shared<TrajSecPntPnt>(m_coll, m_dom, m_site1,
                                                           m_site2, true, arc2);

        const auto traj1 = std::make_shared<Traj>(m_site1, m_site2, true),
                traj2 = std::make_shared<Traj>(m_site1, m_site2, false);

        traj1->insSec(trajSec1);
        traj2->insSec(trajSec2);

        m_trajs.push_back(traj1);
        m_trajs.push_back(traj2);
    }
}

BisecFactory::BisecFactory(const SitePtr &site1, const SitePtr &site2)
: m_site1{site1}
, m_site2{site2}
{
    if (typeid(m_site1) != typeid(m_site2)
        && typeid(m_site2) == typeid(PntSite)) {
        std::swap(m_site1, m_site2);
    }
}

BisecPtr BisecFactory::makeBisec() const {
    if (typeid(*m_site1) == typeid(PntSite)
        && typeid(*m_site2) == typeid(PntSite)) {
        const auto pntSite1 = std::static_pointer_cast<PntSite>(m_site1),
                pntSite2 = std::static_pointer_cast<PntSite>(m_site2);

        return std::make_shared<BisecPntPnt>(pntSite1, pntSite2);
    }

    CGAL_assertion(false);
    return std::make_shared<BisecPntPnt>();
}

#ifdef ENABLE_VIEW

BisecGraphicsItem::BisecGraphicsItem(const BisecPtr &bisec)
: m_bisec{bisec}
{
    if (typeid(*m_bisec) == typeid(BisecPntPnt)) {
        const auto bpp = std::static_pointer_cast<BisecPntPnt>(m_bisec);

        if (m_bisec->site1()->weight() != m_bisec->site2()->weight()) {
            m_circ = bpp->toCircle_2();
            m_boundingRect = Util::boundingRect(m_circ);
        }
    }
}

QRectF BisecGraphicsItem::boundingRect() const {
    return m_boundingRect;
}

void BisecGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*,
                              QWidget*) {       
    if (typeid(*m_bisec) == typeid(BisecPntPnt)) {
        const auto bpp = std::static_pointer_cast<BisecPntPnt>(m_bisec);
        Util::draw(painter, bpp->coll()->arcPnt());
        Util::draw(painter, bpp->dom()->arcPnt());
        
        for (const auto &traj : m_bisec->trajs()) {
            for (const auto &trajSec : traj->secs()) {
                if (typeid(*trajSec) == typeid(TrajSecPntPnt)) {
                    const auto tspp = std::static_pointer_cast<TrajSecPntPnt>(trajSec);

                    if (tspp->isEqual()) {
                        const auto seg = tspp->seg();
                        bool bIsLeft = traj->isLeft();

                        const auto brush = QBrush{bIsLeft ? Qt::red : Qt::blue};
                        painter->setPen(QPen{brush, 1.});
                        painter->setBrush(brush);
                        Util::draw(painter, seg);
                    } else {
                        const auto arc = tspp->arc();
                        bool bIsLeft = traj->isLeft();

                        const auto brush = QBrush{bIsLeft ? Qt::red : Qt::blue};
                        painter->setPen(QPen{brush, 1.});
                        painter->setBrush(brush);
                        Util::draw(painter, arc);
                    }
                }
            }
        }
    }
}

void BisecGraphicsItem::modelChanged() {
    update();
}

void BisecGraphicsItem::onTimeChanged(double t) {
    m_sqrdTime = FT{t};
    update();
}

#include "moc_bisector.cpp"

#endif
