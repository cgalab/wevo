#ifndef SITES_H
#define SITES_H

#include <memory>
#ifdef ENABLE_VIEW
#include <QWidget>
#include <QPainter>
#include <CGAL/Qt/GraphicsItem.h>
#endif
#include "defs.h"

class Site;
class PntSite;

using SitePtr = std::shared_ptr<Site>;
using PntSitePtr = std::shared_ptr<PntSite>;

class Site
: private boost::equivalent<Site>
, boost::totally_ordered<Site> {
public:
    Site(int weight = 0., int id = -1);
    virtual ~Site() = default;

    Site(const Site &site) = default;
    Site & operator=(const Site &site) = default;
    Site(Site &&site) = default;
    Site & operator=(Site &&site) = default;
    
    bool operator<(const Site &other) const;

    const FT & weight() const {
        return m_weight;
    }

    int id() const {
        return m_id;
    }

    double angle(const Circular_arc_point_2 &arcPnt) const;
    virtual Root_of_2 sqrdDist(const Circular_arc_point_2 &arcPnt) const = 0;
    virtual Point_2 center() const = 0;

private:
    FT m_weight;
    int m_id;
};

class PntSite : public Site {
public:
    PntSite(const Point_2 &pnt = Point_2{}, int weight = 0, int id = -1);

    const Point_2 & pnt() const {
        return m_pnt;
    }

    Circle_2 growOffAt(const Root_of_2 &sqrdTime) const;
    Root_of_2 sqrdDist(const Circular_arc_point_2 &arcPnt) const override;

    Point_2 center() const override {
        return pnt();
    }

private:
    Point_2 m_pnt;
};

#ifdef ENABLE_VIEW

class SiteGraphicsItem : public CGAL::Qt::GraphicsItem {
    Q_OBJECT

public:
    SiteGraphicsItem(const SitePtr &site);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = nullptr) override;

public slots:
    void modelChanged() override;
    void onToggle(bool bIsVisible);
    
private:
    SitePtr m_site;
    QRectF m_boundingRect;
    double m_time{0.};
};

#endif

#endif /* SITES_H */
