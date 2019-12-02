#ifndef BISECTOR_H
#define BISECTOR_H

#include "traj.h"

class Bisec;

using BisecPtr = std::shared_ptr<Bisec>;

class Bisec {
public:
    Bisec() = default;
    virtual ~Bisec() = default;

    Bisec(const Bisec &bisec) = default;
    Bisec & operator=(const Bisec &bisec) = default;
    Bisec(Bisec &&bisec) = default;
    Bisec & operator=(Bisec &&bisec) = default;

    virtual const std::vector<TrajPtr> & trajs() const = 0;
    virtual SitePtr site1() const = 0;
    virtual SitePtr site2() const = 0;
    virtual const Line_2 & centerLine() const = 0;
    virtual const TrajPtr & findTraj(const Circular_arc_point_2 &pnt) const = 0;

    std::pair<int, int> id() const;
};

// Note that _site1 is always less than (or equal) _site2. Additionally, _sec1
// is always situated to the left of the vector V from _site1 to _site2.
// Contrary, it is guaranteed that _sec2 lies to the right of V in any case.
class BisecPntPnt : public Bisec {
public:
    BisecPntPnt();
    BisecPntPnt(const PntSitePtr &pntSite1, const PntSitePtr &pntSite2);

    const std::vector<TrajPtr> & trajs() const override {
        return m_trajs;
    }

    SitePtr site1() const override {
        return m_site1;
    }

    SitePtr site2() const override {
        return m_site2;
    }

    const Line_2 & centerLine() const override {
        return m_centerLine;
    }

    const TrajPtr & findTraj(const Circular_arc_point_2 &pnt) const override;

    const TransitPntPtr & coll() const {
        return m_coll;
    }

    const TransitPntPtr & dom() const {
        return m_dom;
    }

    Line_2 toLine_2() const;
    Circle_2 toCircle_2() const;
    
private:
    void compTrajs();

    PntSitePtr m_site1;
    PntSitePtr m_site2;
    std::vector<TrajPtr> m_trajs;
    TransitPntPtr m_coll;
    TransitPntPtr m_dom;
    Line_2 m_centerLine;
};

class BisecFactory {
public:
    BisecFactory(const SitePtr &site1, const SitePtr &site2);

    BisecPtr makeBisec() const;

private:
    SitePtr m_site1;
    SitePtr m_site2;
};

#ifdef ENABLE_VIEW

class BisecGraphicsItem : public CGAL::Qt::GraphicsItem {
    Q_OBJECT

public:
    BisecGraphicsItem(const BisecPtr &bisec);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = nullptr) override;
    
public slots:
    void modelChanged() override;
    void onTimeChanged(double t);

private:
    BisecPtr m_bisec;
    Circle_2 m_circ;
    Root_of_2 m_sqrdTime{0};
    QRectF m_boundingRect;
};

#endif

#endif /* BISECTOR_H */
