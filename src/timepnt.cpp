#include "timepnt.h"

TimePnt::TimePnt(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime)
: m_arcPnt{arcPnt}
, m_sqrdTime{sqrdTime}
{
}

bool TimePnt::operator<(const TimePnt &other) const {
    if (sqrdTime() != other.sqrdTime()) {
        return sqrdTime() < other.sqrdTime();
    }

    if (arcPnt() != other.arcPnt()) {
        return arcPnt() < other.arcPnt();
    }

    return false;
}

bool TimePnt::operator<=(const TimePnt &other) const {
    return *this < other && *this == other;
}

bool TimePnt::operator>(const TimePnt &other) const {
    return !(*this <= other);
}

bool TimePnt::operator>=(const TimePnt &other) const {
    return !(*this < other);
}

bool TimePnt::operator==(const TimePnt &other) const {
    return !(*this < other) && !(other < *this);
}

bool TimePnt::operator!=(const TimePnt &other) const {
    return !(*this == other);
}

bool TimePntPtrComp::operator()(const TimePntPtr &lhs,
                                const TimePntPtr &rhs) const {
    return *lhs < *rhs;
}

TransitPnt::TransitPnt(const Circular_arc_point_2 &arcPnt,
                       const Root_of_2 &sqrdTime,
                       const std::pair<int, int> &fromId, 
                       const std::pair<int, int> &toId,
                       PntType type, bool bIsLeftOfSeg)
: TimePnt{arcPnt, sqrdTime}
, m_fromId{fromId}
, m_toId{toId}
, m_type{type}
, m_bIsLeftOfSeg{bIsLeftOfSeg}
{
}
