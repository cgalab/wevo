#ifndef MWVD_H
#define MWVD_H

#include <queue>
#include "offcirc.h"

class Ev;

using EvPtr = std::shared_ptr<Ev>;

enum class EvType {
    None,
    Coll,
    Edge,
    Dom
};

class Ev {
public:
    Ev(const Circular_arc_point_2 &arcPnt = Circular_arc_point_2{}, 
            const Root_of_2 &sqrdTime = 0., const SitePtr &site = SitePtr{});

    virtual ~Ev() = default;

    Ev(const Ev &ev) = default;
    Ev & operator=(const Ev &ev) = default;
    Ev(Ev &&ev) = default;
    Ev & operator=(Ev &&ev) = default;

    const Circular_arc_point_2 & arcPnt() const {
        return m_arcPnt;
    }

    const Root_of_2 & sqrdTime() const {
        return m_sqrdTime;
    }

    const SitePtr & site() const {
        return m_site;
    }

    virtual EvType type() const = 0;

private:
    Circular_arc_point_2 m_arcPnt;
    Root_of_2 m_sqrdTime;
    SitePtr m_site;
};

class CollEv : public Ev {
public:
    CollEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
            const SitePtr &site, const TrajPtr &traj1, const TrajPtr &traj2, 
            bool bPierces);

    EvType type() const override {
        return EvType::Coll;
    }

    const TrajPtr & traj1() const {
        return m_traj1;
    }

    const TrajPtr & traj2() const {
        return m_traj2;
    }

    bool pierces() const {
        return m_bPierces;
    }

private:
    TrajPtr m_traj1;
    TrajPtr m_traj2;
    bool m_bPierces;
};

class EdgeEv : public Ev {
public:
    EdgeEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
            const SitePtr &site, const MovIsectPtr &isect1, 
            const MovIsectPtr &isect2);

    EvType type() const override {
        return EvType::Edge;
    }

    const MovIsectPtr & isect1() const {
        return m_isect1;
    }

    const MovIsectPtr & isect2() const {
        return m_isect2;
    }

private:
    MovIsectPtr m_isect1;
    MovIsectPtr m_isect2;
};

class DomEv : public Ev {
public:
    DomEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
            const SitePtr &site, const MovIsectPtr &isect1, 
            const MovIsectPtr &isect2);

    EvType type() const override {
        return EvType::Dom;
    }

    const MovIsectPtr & isect1() const {
        return m_isect1;
    }

    const MovIsectPtr & isect2() const {
        return m_isect2;
    }

private:
    MovIsectPtr m_isect1;
    MovIsectPtr m_isect2;
    SitePtr m_other;
};

class EvPtrComp {
public:
    bool operator()(const EvPtr &lhs, const EvPtr &rhs) const {
        if (lhs->sqrdTime() != rhs->sqrdTime()) {
            return lhs->sqrdTime() > rhs->sqrdTime();
        }
        
        return false;
    }
};

class VorDiag {
public:
    VorDiag(const std::string &inFilePath,
            bool bUseOverlay = true,
            const std::string &ipeFilePath = "",
            const std::string &csvFilePath = "",
            bool bEnableView = false);
    
    const std::vector<SitePtr> & sites() const {
        return m_sites;
    }

    const std::vector<Circular_arc_2> & edges() const {
        return m_vorEdges;
    }
    
    const std::vector<Line_arc_2> & segs() const {
        return m_vorSegs;
    }
    
    const std::map<int, OffCircPtr> & offCircs() const {
        return m_offCircs;
    }
    
#ifdef ENABLE_VIEW
    const std::vector<EvPtr> & events() const {
        return m_evs;
    }
#endif
    
private:
    void compColls(const std::vector<std::set<int>> &candSets);
    void compColls();
    void compColl(const SitePtr &site1, const SitePtr &site2);
    void run();

    void handleEv(const std::shared_ptr<CollEv> &collEv);
    bool isValidCollEv(const std::shared_ptr<CollEv> &collEv1,
            const std::shared_ptr<CollEv> &collEv2) const;
    bool isValidDomEv(const std::shared_ptr<DomEv> &domEv1,
            const std::shared_ptr<DomEv> &domEv2) const;
    void processEv(const std::shared_ptr<CollEv> &collEv, bool bDom,
            bool bIsValid, bool bPierces);

    void handleEv(const std::shared_ptr<DomEv> &domEv);
    void processEv(const std::shared_ptr<DomEv> &domEv, bool bDom);

    void handleEv(const std::shared_ptr<EdgeEv> &edgeEv);
    bool deleteLowestArc(const std::shared_ptr<EdgeEv> &edgeEv,
            const SitePtr &lowSite, const SitePtr &medSite, const SitePtr &highSite);

    SitePtr otherSite(const SitePtr &site, const MovIsectPtr &isect) const;
    MovIsectPtr makeMovIsectAt(const Circular_arc_point_2 &pnt,
            const SitePtr &site1, const SitePtr &site2);
    MovIsectPtr makeMovIsect(const TrajPtr &traj);

    void checkEdgeEv(const OffCircPtr &offCirc, const Root_of_2 &t,
            const MovIsectPtr &isect, bool bLeft);
    void checkEdgeEv(const OffCircPtr &offCirc, const Root_of_2 &t,
            const MovIsectPtr &isect, const MovIsectPtr &other);
    void compVorEdges();
    void writeToIpe(const std::string &filePath) const;
    void writeToCsv(const std::string &filePath, long long runtime,
            long long overlayDuration, long long eventDuration,
            size_t maxCandSetSize, size_t nCandSets, size_t avgSize) const;
    
    void storeEv(const EvPtr &ev);

    std::priority_queue<EvPtr, std::vector<EvPtr>, EvPtrComp> m_queue;
    std::map<std::pair<int, int>, BisecPtr> m_bisecs;
    std::map<int, OffCircPtr> m_offCircs;
    std::vector<SitePtr> m_sites;
    std::map<MovIsectId, MovIsectPtr> m_isects;
    std::vector<Circular_arc_2> m_vorEdges;
    std::vector<Line_arc_2> m_vorSegs;
    // Used for logging:
    int m_collEvCnt{0};
    int m_invalidCollEvCnt{0};
    int m_domEvCnt{0};
    int m_invalidDomEvCnt{0};
    int m_edgeEvCnt{0};
    int m_invalidEdgeEvCnt{0};
    int m_nVorVerts{0};
#ifdef ENABLE_VIEW
    std::vector<EvPtr> m_evs;
#endif
};

#ifdef ENABLE_VIEW

class VorDiagGraphicsItem : public CGAL::Qt::GraphicsItem {
    Q_OBJECT

public:
    VorDiagGraphicsItem(const VorDiag &vorDiag);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = nullptr) override;
    
public slots:
    void modelChanged() override;
    void onToggle(bool bIsVisible);
    void onPrevEv(double t);
    void onNextEv(double t);

signals:
    void timeChanged(double t);

private:
    VorDiag m_vorDiag;
    Root_of_2 m_sqrdTime{0};
    QRectF m_boundingRect;
    bool m_bShow{true};
    size_t m_evIndex{0};
    QPointF m_evPnt;
    double m_time{0.};
};

#endif

#endif /* MWVD_H */
