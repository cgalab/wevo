#include "traj.h"
#include "log.h"

TrajSec::TrajSec(const TransitPntPtr &start, const TransitPntPtr &end)
: m_start{start}
, m_end{end}
{
}

std::pair<int, int> TrajSec::id() const {
    return std::minmax({site1()->id(), site2()->id()});
}

bool TrajSec::incls(const Root_of_2 &sqrdTime) const {
    return m_start->sqrdTime() <= sqrdTime && sqrdTime <= m_end->sqrdTime();
}

TrajSecPntPnt::TrajSecPntPnt(const TransitPntPtr &start, const TransitPntPtr &end,
                             const PntSitePtr &site1, const PntSitePtr &site2, 
                             bool bLeftIsect, const Circular_arc_2 &arc)
: TrajSec(start, end)
, m_site1{site1}
, m_site2{site2}
, m_bLeftIsect{bLeftIsect}
, m_arc{arc}
{
    CGAL_assertion(m_site1->weight() > m_site2->weight());
    m_bIsEqual = false;
}

TrajSecPntPnt::TrajSecPntPnt(const TransitPntPtr &start, const TransitPntPtr &end,
                             const PntSitePtr &site1, const PntSitePtr &site2,
                             bool bLeftIsect, const Line_arc_2 &seg)
: TrajSec(start, end)
, m_site1{site1}
, m_site2{site2}
, m_bLeftIsect{bLeftIsect}
, m_seg{seg} {
    CGAL_assertion(m_site1->weight() == m_site2->weight());
    m_bIsEqual = true; 
}

Circular_arc_point_2 TrajSecPntPnt::pntAt(const Root_of_2 &sqrdTime) const {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("pntAt");
    src::severity_logger<severity_level> slg;
#endif
    
    // TODO: Optimize! Create a specialized checkNeighbor function!
    // Equality check is potentially expensive!
    if (sqrdTime == start()->sqrdTime()) {
        return start()->arcPnt();
    }
    
    if (sqrdTime == end()->sqrdTime()) {
        return end()->arcPnt();
    }
    
    const auto &circ1 = m_site1->growOffAt(sqrdTime);

    if (!isEqual()) {
        using IsectResult
                = CGAL::CK2_Intersection_traits<CK, Circular_arc_2, Circle_2>::type;
        using Isect = std::pair<Circular_arc_point_2, unsigned int>;
        std::vector<IsectResult> results;
        CGAL::intersection(m_arc, circ1, std::back_inserter(results));

        CGAL_assertion(results.size() < 2);
        if (results.size() == 1) {
            const auto isect = std::get<0>(boost::get<Isect>(results.at(0)));
            return isect;
        }
    } else {
        using IsectResult
                = CGAL::CK2_Intersection_traits<CK, Line_arc_2, Circle_2>::type;
        using Isect = std::pair<Circular_arc_point_2, unsigned int>;
        std::vector<IsectResult> results;
        CGAL::intersection(m_seg, circ1, std::back_inserter(results));

        CGAL_assertion(results.size() < 2);
        if (results.size() == 1) {
            const auto isect = std::get<0>(boost::get<Isect>(results.at(0)));
            return isect;
        }
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, error) << "No point found at time "
            << std::sqrt(CGAL::to_double(sqrdTime)) << ".";
    BOOST_LOG_SEV(slg, error) << "Time interval equals ["
            << std::sqrt(CGAL::to_double(start()->sqrdTime())) << ", "
            << std::sqrt(CGAL::to_double(end()->sqrdTime())) << "].";
#endif
    
    return Circular_arc_point_2{};
}

/*TrajSecFactory::TrajSecFactory(const SitePtr &site1, const SitePtr &site2,
                               const TransitPntPtr &start, 
                               const TransitPntPtr &end, bool bLeftIsect,
                               bool bIsLeftOfSeg)
: m_site1{site1}
, m_site2{site2}
, m_start{start}
, m_end{end}
, m_bLeftIsect{bLeftIsect}
, m_bIsLeftOfSeg{bIsLeftOfSeg}
{
}

TrajSecPtr TrajSecFactory::makeTrajSec() {
    if (typeid(m_site1) == typeid(PntSite)
        && typeid(m_site2) == typeid(PntSite)) {
        const auto pntSite1 = std::static_pointer_cast<PntSite>(_site1),
                pntSite2 = std::static_pointer_cast<PntSite>(_site2);
        const auto circ = Bisec::toCircle_2(pntSite1, pntSite2);
        const auto arc = Circular_arc_2{circ.center(), _bLeftIsect ? _end->pnt() : _start->pnt(),
                               _bLeftIsect ? _start->pnt() : _end->pnt()};

        return std::make_shared<TrajSecPntPnt>(_start, _end, pntSite1, pntSite2,
                                               _bLeftIsect, arc);
    }
}*/

Traj::Traj() {
}

Traj::Traj(const Traj &other)
: m_site1{other.site1()}
, m_site2{other.site2()}
, m_bIsLeft{other.isLeft()}
, m_bIsFirst{other.isFirst()}
, m_trajSecs{other.secs()}
{
}

Traj::Traj(const SitePtr &site1, const SitePtr &site2, bool bIsLeft, bool bIsFirst,
           const std::vector<TrajSecPtr> &trajSecs)
: m_site1{site1}
, m_site2{site2}
, m_bIsLeft{bIsLeft}
, m_bIsFirst{bIsFirst}
, m_trajSecs{trajSecs}
{
}

bool Traj::operator==(const Traj &other) const {
    return m_site1->id() == other.site1()->id() && m_site2->id() == other.site2()->id()
            && m_bIsLeft == other.isLeft() && m_bIsFirst == other.isFirst();
}

std::pair<int, int> Traj::id() const {
    return std::minmax({site1()->id(), site2()->id()});
}

Circular_arc_point_2 Traj::pntAt(const Root_of_2 &sqrdTime) const {
    for (const auto &trajSec : m_trajSecs) {
        if (trajSec->incls(sqrdTime)) {
            return trajSec->pntAt(sqrdTime);
        }
    }
    
    return Point_2{};
}

TransitPntPtr Traj::start() const {
    CGAL_assertion(!m_trajSecs.empty());
    if (!m_trajSecs.empty()) {
        return m_trajSecs.front()->start();
    }

    return std::make_shared<TransitPnt>();
}

TransitPntPtr Traj::end() const {
    CGAL_assertion(!m_trajSecs.empty());
    if (!m_trajSecs.empty()) {
        return m_trajSecs.back()->end();
    }

    return std::make_shared<TransitPnt>();
}

int Traj::isect(std::vector<TimePntPtr> &timePnts, const TrajPtr &other) const {
    CGAL_assertion(m_site1->id() == other->site1()->id()
           || m_site1->id() == other->site2()->id()
           || m_site2->id() == other->site1()->id()
           || m_site2->id() == other->site2()->id());

    if (id() == other->id()) {
        return 0;
    }
    
    size_t n = 0;
    for (const auto &sec1 : secs()) {
        for (const auto &sec2 : other->secs()) {
            if (typeid(*sec1) == typeid(TrajSecPntPnt)
                && typeid(*sec2) == typeid(TrajSecPntPnt)) {
                auto tspp1 = std::static_pointer_cast<TrajSecPntPnt>(sec1),
                        tspp2 = std::static_pointer_cast<TrajSecPntPnt>(sec2);

                if (tspp1->isEqual() || tspp2->isEqual()) {
                    if (!tspp1->isEqual()) {
                        std::swap(tspp1, tspp2);
                    }
                    
                    if (!tspp2->isEqual()) {
                        const auto seg = tspp1->seg();
                        const auto arc = tspp2->arc();

                        using IsectResult
                                = CGAL::CK2_Intersection_traits<CK, Line_arc_2, Circular_arc_2>::type;
                        using Isect = std::pair<Circular_arc_point_2, unsigned int>;
                        std::vector<IsectResult> results;
                        CGAL::intersection(seg, arc, std::back_inserter(results));

                        for (const auto &result : results) {
                            const auto arcPnt = std::get<0>(boost::get<Isect>(result));
                            const auto sqrdDist = m_site1->sqrdDist(arcPnt);
                            timePnts.push_back(std::make_shared<TimePnt>(arcPnt, sqrdDist));
                            n++;
                        }
                    } else {
                        const auto seg1 = tspp1->seg(), seg2 = tspp2->seg();

                        using IsectResult
                                = CGAL::CK2_Intersection_traits<CK, Line_arc_2, Line_arc_2>::type;
                        using Isect = std::pair<Circular_arc_point_2, unsigned int>;
                        std::vector<IsectResult> results;
                        CGAL::intersection(seg1, seg2, std::back_inserter(results));

                        for (const auto &result : results) {
                            const auto arcPnt = std::get<0>(boost::get<Isect>(result));
                            const auto sqrdDist = m_site1->sqrdDist(arcPnt);
                            timePnts.push_back(std::make_shared<TimePnt>(arcPnt, sqrdDist));
                            n++;
                        }
                    }
                } else {
                    CGAL_assertion(!tspp1->isEqual() && !tspp2->isEqual());
                    const auto arc1 = tspp1->arc(), arc2 = tspp2->arc();

                    using IsectResult
                            = CGAL::CK2_Intersection_traits<CK, Circular_arc_2,
                            Circular_arc_2>::type;
                    using Isect = std::pair<Circular_arc_point_2, unsigned int>;
                    std::vector<IsectResult> results;
                    CGAL::intersection(arc1, arc2, std::back_inserter(results));

                    for (const auto &result : results) {
                        const auto arcPnt = std::get<0>(boost::get<Isect>(result));
                        const auto sqrdDist = m_site1->sqrdDist(arcPnt);
                        timePnts.push_back(std::make_shared<TimePnt>(arcPnt, sqrdDist));
                        n++;
                    }
                }
            }
        }
    }

    return n;
}

const SitePtr & Traj::getOtherSite(int siteId) const {
    CGAL_assertion(id().first == siteId || id().second == siteId);
    const auto otherSiteId = id().first == siteId ? id().second : id().first;
    return m_site1->id() == otherSiteId ? m_site1 : m_site2;
}
