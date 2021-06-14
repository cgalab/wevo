#ifndef OFFCIRC_H
#define OFFCIRC_H

#include <map>
#include <set>
#include "isect.h"
#include "file_io.h"

class OffCirc;

using OffCircPtr = std::shared_ptr<OffCirc>;
using ArcAction 
        = std::tuple<Root_of_2, bool, std::pair<MovIsectId, MovIsectId>, bool>;

class OffCirc {
public:
    OffCirc(const SitePtr &site);

    bool isActive() const {
        return m_bIsActive;
    }

    const SitePtr & site() const {
        return m_site;
    }

    const std::map<std::pair<MovIsectId, MovIsectId>, bool> & arcs() const {
        return m_arcs;
    }

    const std::map<MovIsectId, MovIsectPtr> & isects() const {
        return m_isects;
    }

#ifdef ENABLE_VIEW
    const std::vector<ArcAction> & arcActions() const {
        return m_arcActions;
    }
    
    const std::map<MovIsectId, MovIsectPtr> & isectHistory() const {
        return m_isectHistory;
    }
    
    const Root_of_2 & domSqrdTime() const {
        return m_domSqrdTime;
    }

    void writeArcsToIpe(IpeWriter &iw, const Root_of_2 &maxTime,
            const Root_of_2 &timeStep, double scale,
            const std::vector<Root_of_2> &evSqrdTimes) const;
#endif
    
    bool neighbor(MovIsectPtr &neighbor, const MovIsectPtr &isect, bool bLeft) const;

    void spawnArc(const Root_of_2 &sqrdTime, const MovIsectPtr &isect1, const MovIsectPtr &isect2,
            bool bIsActive, bool bPierces = false);
    bool deleteArc(std::pair<MovIsectPtr, MovIsectPtr> &newArc, const Root_of_2 &sqrdTime,
            const MovIsectPtr &from, const MovIsectPtr &to, bool bIsActive);
    bool deleteArcUnordered(std::pair<MovIsectPtr, MovIsectPtr> &newArc,
            const Root_of_2 &sqrdTime, const MovIsectPtr &isect1, 
            const MovIsectPtr &isect2, bool bIsActive);

    bool collapseArc(const Root_of_2 &sqrdTime, const MovIsectPtr &from,
            const MovIsectPtr &to);
    std::pair<bool, bool> expandIsect(const Root_of_2 &sqrdTime,
            const Circular_arc_point_2 &evPnt, const MovIsectPtr &from,
            const MovIsectPtr &to, bool bIsOnWf);
    bool replaceIsect(const Root_of_2 &sqrdTime, const MovIsectPtr &oldIsect,
            const MovIsectPtr &newIsect);

    bool searchNeighbor(MovIsectPtr &neighbor, const MovIsectPtr &isect,
            bool bLeft) const;
    bool searchNeighbor(MovIsectPtr &neighbor, const Root_of_2 &sqrdTime,
            const Circular_arc_point_2 &pnt, bool bLeft) const;

    bool inclsIsect(const MovIsectPtr &isect) const;
    bool isInActiveArc(const Root_of_2 &sqrdTime,
            const Circular_arc_point_2 &pnt) const;

#ifdef ENABLE_LOGGING
    void logInfo(const Root_of_2 &sqrdTime) const;
#endif

private:
    bool searchNeighbor(MovIsectPtr &neighbor, const Root_of_2 &sqrdTime,
            double angle1, bool bLeft) const;

    void insertArc(const Root_of_2 &sqrdTime,
            const std::pair<MovIsectId, MovIsectId> &arcId, bool bIsOnWf);
    void insertArc(const Root_of_2 &sqrdTime, const MovIsectId &fromId,
            const MovIsectId &to, bool bIsOnWf);

    void eraseArc(const Root_of_2 &sqrdTime,
            const std::pair<MovIsectId, MovIsectId> &arcId, bool bAssert = true);
    void eraseArc(const Root_of_2 &sqrdTime, const MovIsectId &fromId,
            const MovIsectId &toId, bool bAssert = true);

    void insertIsect(const MovIsectPtr &isect);
    
    bool inclsArc(const std::pair<MovIsectId, MovIsectId> &arcId,
            bool bAssert) const;

    bool m_bIsActive{true};
    SitePtr m_site;
    std::map<std::pair<MovIsectId, MovIsectId>, bool> m_arcs;
    std::map<MovIsectId, MovIsectPtr> m_isects;
    // Stores the left and right neighbor of a moving intersection.
    std::map<MovIsectId, MovIsectId> m_lefts;
    std::map<MovIsectId, MovIsectId> m_rights;
#ifdef ENABLE_VIEW
    std::vector<ArcAction> m_arcActions;
    std::map<MovIsectId, MovIsectPtr> m_isectHistory;
    Root_of_2 m_domSqrdTime{-1};
#endif
};

#ifdef ENABLE_VIEW

class OffCircGraphicsItem : public CGAL::Qt::GraphicsItem {
    Q_OBJECT

public:
    OffCircGraphicsItem(const OffCircPtr &offCirc);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = nullptr) override;
    
public slots:
    void modelChanged() override;
    void onTimeChanged(double t);
    void onToggle(bool bIsVisible);
    
private:
    void updateBoundingRect(double offset = 2.);
    
    OffCircPtr m_offCirc;
    QPointF m_center;
    Root_of_2 m_sqrdTime{0};
    QRectF m_boundingRect;
    size_t m_step{0};
    std::map<std::pair<MovIsectId, MovIsectId>, bool> m_arcs;
    std::map<MovIsectId, size_t> m_switches;
    bool m_bIncreased{false};
    bool m_bIsVisible{true};
};

#endif

#endif /* OFFCIRC_H */
