#ifndef ISECT_H
#define ISECT_H

#include "bisector.h"

class MovIsect;

using MovIsectPtr = std::shared_ptr<MovIsect>;
using MovIsectId = std::tuple<int, int, bool, bool>;

// An object of type MovIsect models a moving intersection point that
// travels along a particular bisector section.
class MovIsect {
public:
    friend std::ostream & operator<<(std::ostream &os, const MovIsect &isect);

    MovIsect(const TrajPtr &traj);

    bool operator<(const MovIsect &other) const;

    Circular_arc_point_2 pntAt(const Root_of_2 &sqrdTime) const;

    const MovIsectId id() const;

    const TrajPtr & traj() const {
        return m_traj;
    }

    const std::vector<std::pair<Root_of_2, bool>> &switches() const {
        return m_switches;
    }

    bool isWfVert() const {
        return m_bIsWfVert;
    }

    void setIsWfVert(const Root_of_2 &sqrdTime, bool bIsWfVert);
    
    std::string toString() const;

private:
    // Holds the bisector section along which the wavefront corner travels.
    TrajPtr m_traj;
    bool m_bIsWfVert{true};
    std::vector<std::pair<Root_of_2, bool>> m_switches;
};

#endif /* ISECT_H */
