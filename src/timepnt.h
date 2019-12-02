#ifndef TIMEPNT_H
#define TIMEPNT_H

#include "sites.h"

class TimePnt;
class TransitPnt;

using TimePntPtr = std::shared_ptr<TimePnt>;
using TransitPntPtr = std::shared_ptr<TransitPnt>;

enum class PntType {
    None,
    Time,
    Trans,
    Coll,
    Dom
};

class TimePnt {
public:
    TimePnt(const Circular_arc_point_2 &pnt = Circular_arc_point_2{}, 
            const Root_of_2 &sqrdTime = 0.);
    virtual ~TimePnt() = default;

    TimePnt(const TimePnt &timePnt) = default;
    TimePnt & operator=(const TimePnt &timePnt) = default;
    TimePnt(TimePnt &&timePnt) = default;
    TimePnt & operator=(TimePnt &&timePnt) = default;

    bool operator<(const TimePnt &other) const;
    bool operator<=(const TimePnt &other) const;
    bool operator>(const TimePnt &other) const;
    bool operator>=(const TimePnt &other) const;
    bool operator==(const TimePnt &other) const;
    bool operator!=(const TimePnt &other) const;
    
    const Circular_arc_point_2 & arcPnt() const {
        return m_arcPnt;
    }

    const Root_of_2 & sqrdTime() const {
        return m_sqrdTime;
    }
    
    void setArcPnt(const Circular_arc_point_2 &arcPnt) {
        m_arcPnt = arcPnt;
    }

    void setSqrdTime(const Root_of_2 &sqrdTime) {
        m_sqrdTime = sqrdTime;
    }
    
    virtual PntType type() const {
        return PntType::Time;
    };

private:
    Circular_arc_point_2 m_arcPnt;
    Root_of_2 m_sqrdTime;
};

class TimePntPtrComp {
public:
    bool operator()(const TimePntPtr &lhs, const TimePntPtr &rhs) const;
};

class TransitPnt : public TimePnt {
public:
    TransitPnt(const Circular_arc_point_2 &pnt = Circular_arc_point_2{}, 
            const Root_of_2 &sqrdTime = 0.,
            const std::pair<int, int> &fromId = {-1, -1},
            const std::pair<int, int> &toId = {-1, -1},
            PntType type = PntType::Trans,
            bool bIsLeftOfSeg = false);

    const std::pair<int, int> & fromId() const {
        return m_fromId;
    }

    const std::pair<int, int> & toId() const {
        return m_toId;
    }
    
    PntType type() const override {
        return m_type;
    }

    bool isLeftOfSeg() const {
        return m_bIsLeftOfSeg;
    }

    void setFromId(const std::pair<int, int> &fromId) {
        m_fromId = fromId;
    }

    void setToId(const std::pair<int, int> &toId) {
        m_toId = toId;
    }

    void setType(PntType type) {
        m_type = type;
    }

    void setIsLeftOfSeg(bool bLeftOfSeg) {
        m_bIsLeftOfSeg = bLeftOfSeg;
    }

private:
    std::pair<int, int> m_fromId;
    std::pair<int, int> m_toId;
    PntType m_type;
    bool m_bIsLeftOfSeg;
};

#endif /* TIMEPNT_H */

