#ifndef TRAJ_H
#define TRAJ_H

#include "timepnt.h"

class Traj;
class TrajSec;

using TrajPtr = std::shared_ptr<Traj>;
using TrajSecPtr = std::shared_ptr<TrajSec>;

class TrajSec {
public:
    TrajSec() = default;
    virtual ~TrajSec() = default;

    TrajSec(const TrajSec &trajSec) = default;
    TrajSec & operator=(const TrajSec &trajSec) = default;
    TrajSec(TrajSec &&trajSec) = default;
    TrajSec & operator=(TrajSec &&trajSec) = default;

    TrajSec(const TransitPntPtr &start = TransitPntPtr{},
    const TransitPntPtr &end = TransitPntPtr{});

    const TransitPntPtr & start() const {
        return m_start;
    }

    const TransitPntPtr & end() const {
        return m_end;
    }

    virtual SitePtr site1() const = 0;
    virtual SitePtr site2() const = 0;
    virtual Circular_arc_point_2 pntAt(const Root_of_2 &sqrdTime) const = 0;

    std::pair<int, int> id() const;
    bool incls(const Root_of_2 &sqrdTime) const;

private:
    TransitPntPtr m_start;
    TransitPntPtr m_end;
};

class TrajSecPntPnt : public TrajSec {
public:
    TrajSecPntPnt(const TransitPntPtr &start, const TransitPntPtr &end,
                  const PntSitePtr &site1, const PntSitePtr &site2,
                  bool bLeftIsect, const Circular_arc_2 &arc);

    TrajSecPntPnt(const TransitPntPtr &start, const TransitPntPtr &end,
                  const PntSitePtr &site1, const PntSitePtr &site2,
                  bool bLeftIsect, const Line_arc_2 &seg);

    SitePtr site1() const override {
        return m_site1;
    }

    SitePtr site2() const override {
        return m_site2;
    }

    bool leftIsect() const {
        return m_bLeftIsect;
    }

    const Circular_arc_2 & arc() const {
        return m_arc;
    }
    
    const Line_arc_2 & seg() const {
        return m_seg;
    }

    bool isEqual() const {
        return m_bIsEqual;
    }
    
    Circular_arc_point_2 pntAt(const Root_of_2 &sqrdTime) const override;

private:
    PntSitePtr m_site1;
    PntSitePtr m_site2;
    bool m_bLeftIsect;
    Circular_arc_2 m_arc;
    Line_arc_2 m_seg;
    bool m_bIsEqual;
};

class TrajSecPtrComp {
public:
    bool operator()(const TrajSecPtr &lhs, const TrajSecPtr &rhs) const;
};

/*class TrajSecFactory {
public:
    TrajSecFactory(const SitePtr &site1, const SitePtr &site2,
            const TransitPntPtr &start, const TransitPntPtr &end,
            bool bLeftIsect, bool bIsLeftOfSeg);

    TrajSecPtr makeTrajSec();

private:
    SitePtr m_site1;
    SitePtr m_site2;
    TransitPntPtr m_start;
    TransitPntPtr m_end;
    bool m_bLeftIsect;
    bool m_bIsLeftOfSeg;
};*/

class Traj {
public:
    Traj();
    Traj(const Traj &other);
    Traj(const SitePtr &site1, const SitePtr &site2, bool bIsLeft, bool bIsFirst = true,
            const std::vector<TrajSecPtr> &trajSecs = std::vector<TrajSecPtr>());

    bool operator==(const Traj &other) const;

    const std::vector<TrajSecPtr> & secs() const {
        return m_trajSecs;
    }

    const SitePtr & site1() const {
        return m_site1;
    }

    const SitePtr & site2() const {
        return m_site2;
    }

    bool isLeft() const {
        return m_bIsLeft;
    }

    bool isFirst() const {
        return m_bIsFirst;
    }

    std::pair<int, int> id() const;
    Circular_arc_point_2 pntAt(const Root_of_2 &sqrdTime) const;
    TransitPntPtr start() const;
    TransitPntPtr end() const;

    int isect(std::vector<TimePntPtr> &timePnts, const TrajPtr &other) const;

    void insSec(const TrajSecPtr &sec) {
        m_trajSecs.push_back(sec);
    }

private:
    SitePtr m_site1;
    SitePtr m_site2;
    bool m_bIsLeft{false};
    bool m_bIsFirst{true};
    std::vector<TrajSecPtr> m_trajSecs;
};

#endif /* TRAJ_H */
