#include <iostream>
#include <cmath>
#include <algorithm>
#include "isect.h"

MovIsect::MovIsect(const TrajPtr &sec)
: m_traj{sec}
{
}

std::ostream & operator<<(std::ostream &os, const MovIsect &isect) {
    std::cout << isect.toString();

    return os;
}

bool MovIsect::operator<(const MovIsect &other) const {
    return id() < other.id();
}

Circular_arc_point_2 MovIsect::pntAt(const Root_of_2 &sqrdTime) const {
    return m_traj->pntAt(sqrdTime);
}

const MovIsectId MovIsect::id() const {
    const auto &id = std::minmax({m_traj->site1()->id(), m_traj->site2()->id()});
    return std::make_tuple(id.first, id.second, m_traj->isLeft(), m_traj->isFirst());
}

void MovIsect::setIsWfVert(const Root_of_2 &sqrdTime, bool bIsWfVert) {
    m_switches.push_back(std::make_pair(sqrdTime, bIsWfVert));
    m_bIsWfVert = bIsWfVert;
}

std::string MovIsect::toString() const {
    std::stringstream strs;
    strs << std::boolalpha << "(" << std::get<0>(id()) << ", " 
            << std::get<1>(id()) << ", " << std::get<2>(id()) 
            << ", " << std::get<3>(id()) << ")";

    return strs.str();
}
