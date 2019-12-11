#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "log.h"
#include "offcirc.h"
#include "util.h"

OffCirc::OffCirc(const SitePtr &site)
: m_site{site}
{
}

#ifdef ENABLE_VIEW

void OffCirc::writeArcsToIpe(IpeWriter &iw, const Root_of_2 &maxTime, 
                             const Root_of_2 &timeStep, double scale,
                             const std::vector<Root_of_2> &evSqrdTimes) const {
    std::map<std::pair<MovIsectId, MovIsectId>, bool> arcs;
    std::map<MovIsectId, size_t> switches;
    size_t step = 0, i1 = 1, i2 = 1;

    const auto fLayer
            = [&](int id) {
                std::stringstream strs;
                switch (id) {
                    case 0:
                    {
                        strs << "Wavefront" << i1;
                        return strs.str();
                    }
                    case 1:
                    {
                        strs << "Active" << i1;
                        return strs.str();
                    }
                    case 2:
                    {
                        strs << "Event" << i2;
                        return strs.str();
                    }
                    default:
                    {
                        return std::string{};
                    }
                }
            };
    
    const auto fAdvanceArcs
            = [&](const Root_of_2 &time, size_t step) {
                auto nextAction = m_arcActions.at(step);
                auto nextSqrdTime = std::get<0>(nextAction);
            
                while (nextSqrdTime < time) {
                    bool bInsert = std::get<1>(nextAction);
                    const auto arcId = std::get<2>(nextAction);
                    const auto isect1 = m_isectHistory.at(arcId.first),
                            isect2 = m_isectHistory.at(arcId.second);
                    size_t si1 = switches.at(isect1->id()),
                            si2 = switches.at(isect2->id());

                    if (si1 < isect1->switches().size() - 1) {
                        const auto nsw = isect1->switches().at(si1);
                        if (nsw.first < time) {
                            switches[isect1->id()] = si1 + 1;
                        }
                    }

                    if (si2 < isect2->switches().size() - 1) {
                        const auto nsw = isect2->switches().at(si2);
                        if (nsw.first < time) {
                            switches[isect2->id()] = si2 + 1;
                        }
                    }

                    if (bInsert) {
                        arcs[arcId] = std::get<3>(nextAction);
                    } else if (arcs.find(arcId) != arcs.end()) {
                        arcs.erase(arcs.find(arcId));
                    }

                    if (step + 1 < m_arcActions.size()) {
                        nextAction = m_arcActions.at(++step);
                        nextSqrdTime = std::get<0>(nextAction);
                    } else {
                        ++step;
                        break;
                    }
                }
            };

    const auto fWriteArcs
            = [&](const Root_of_2 &time, bool bIsEv) {
                if (arcs.empty()
                    && (time < m_domSqrdTime || m_domSqrdTime < 0)) {
                    if (typeid(*m_site) == typeid(PntSite)) {
                        const auto pntSite
                                = std::static_pointer_cast<PntSite>(m_site);
                        const auto circ = pntSite->growOffAt(time);
                        iw.addIpeObj(IpeObj{circ,
                                     fLayer(bIsEv ? 2 : 0),
                                     scale,
                                     "blue"});
                    }
                }

                for (const auto &arc : arcs) {
                    const auto arcId = arc.first;
                    bool bIsOnWf = arc.second;

                    const auto isect1 = m_isectHistory.at(arcId.first),
                            isect2 = m_isectHistory.at(arcId.second);
                    const auto &pnt1 = isect1->pntAt(time),
                            &pnt2 = isect2->pntAt(time);
                    bool bIsWfVert1 = false, bIsWfVert2 = false;

                    if (!switches.empty()) {
                        size_t si1 = switches.at(isect1->id()),
                                si2 = switches.at(isect2->id());
                        const auto sw1 = isect1->switches(), 
                                sw2 = isect2->switches();

                        if (si1 > 0) {
                            bIsWfVert1 = sw1.at(si1 - 1).second;
                        }

                        if (si2 > 0) {
                            bIsWfVert2 = sw2.at(si2 - 1).second;
                        }
                    }


                    if (typeid(*m_site) == typeid(PntSite)) {
                        const auto pntSite
                                = std::static_pointer_cast<PntSite>(m_site);
                        const auto circ = pntSite->growOffAt(time);
                        const auto arc = Circular_arc_2{circ, pnt1, pnt2};
                        iw.addIpeObj(IpeObj{arc,
                                            fLayer(bIsEv ? 2 : (bIsOnWf ? 0 : 1)),
                                            scale,
                                            bIsOnWf ? "blue" : "orange"});
                    }

                    iw.addIpeObj(IpeObj{pnt1,
                                        fLayer(bIsEv ? 2 : (bIsWfVert1 ? 0 : 1)),
                                        scale,
                                        bIsWfVert1 ? "blue" : "orange"});
                    iw.addIpeObj(IpeObj{pnt2,
                                        fLayer(bIsEv ? 2 : (bIsWfVert2 ? 0 : 1)),
                                        scale,
                                        bIsWfVert2 ? "blue" : "orange"});
                }
                
                bIsEv ? i2++ : i1++;
            };
            
    for (const auto &isect : m_isectHistory) {
        switches[isect.first] = 0;
    }
    
    for (Root_of_2 time = timeStep; time < maxTime; time += timeStep) {
        if (step < m_arcActions.size()) {
            fAdvanceArcs(CGAL::square(time), step);
            fWriteArcs(CGAL::square(time), false);
        }
    }

    arcs.clear();
    switches.clear();
    step = 0; i1 = 1; i2 = 1;
    
    for (const auto &isect : m_isectHistory) {
        switches[isect.first] = 0;
    }
    
    for (const auto &evSqrdTime : evSqrdTimes) {
        if (step < m_arcActions.size()) {
            fAdvanceArcs(evSqrdTime, step);
            fWriteArcs(evSqrdTime, true);
        }
    }
}

#endif

bool OffCirc::neighbor(MovIsectPtr &neighbor, const MovIsectPtr &isect,
                       bool bLeft) const {
    bool bFound = bLeft ? m_lefts.find(isect->id()) != m_lefts.end()
            : m_rights.find(isect->id()) != m_rights.end();
    if (bFound) {
        const auto id = bLeft ? m_lefts.at(isect->id()) : m_rights.at(isect->id());

        CGAL_assertion(m_isects.find(id) != m_isects.end());
        if (m_isects.find(id) != m_isects.end()) {
            neighbor = m_isects.at(id);
            return true;
        }
    }

    return false;
}

void OffCirc::spawnArc(const Root_of_2 &sqrdTime, const MovIsectPtr &isect1,
                       const MovIsectPtr &isect2, bool bIsActive, bool bPierces) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("spawnArc");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Before:";
    logInfo(sqrdTime);
#endif
        
    if (!m_bIsActive) {
        return;
    }

    auto from = bIsActive ? isect1 : isect2,
            to = bIsActive ? isect2 : isect1;
    
    if (!bIsActive && bPierces) {
        std::swap(from, to);
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "Inserting arc " << from->toString()
            << " -> " << to->toString() << " ...";
#endif

    if (m_isects.empty()) {
        if (bIsActive) {
            insertArc(sqrdTime, from->id(), to->id(), false);
        }
        insertArc(sqrdTime, to->id(), from->id(), true);

        insertIsect(isect1);
        isect1->setIsWfVert(sqrdTime, true);

        insertIsect(isect2);
        isect2->setIsWfVert(sqrdTime, true);

#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, normal) << "After:";
        logInfo(sqrdTime);
#endif
        
        return;
    }

    const auto pnt = isect1->traj()->start()->arcPnt();
    MovIsectPtr left, right;
    bool bOk1 = searchNeighbor(left, sqrdTime, pnt, true),
            bOk2 = searchNeighbor(right, sqrdTime, pnt, false);

#ifdef ENABLE_LOGGING
    if (!bOk1) {
        BOOST_LOG_SEV(slg, error) << "Could not find left neighbor of"
                << isect1->toString() << "!";
    }

    if (!bOk2) {
        BOOST_LOG_SEV(slg, error) << "Could not find right neighbor of"
                << isect2->toString() << "!";
    }
#endif
    
    CGAL_assertion(bOk1 && bOk2);
    if (bOk1 && bOk2) {
        const auto arcId = std::make_pair(left->id(), right->id());

        if (inclsArc(arcId, true)) {
            bool bIsOnWf = m_arcs.at(arcId);
            
            eraseArc(sqrdTime, arcId);
            insertArc(sqrdTime, left->id(), from->id(), bIsOnWf);
            
            if (bIsActive) {
                insertArc(sqrdTime, from->id(), to->id(), bPierces);
            }
            
            insertArc(sqrdTime, to->id(), right->id(), bIsOnWf);

            insertIsect(isect1);
            isect1->setIsWfVert(sqrdTime, bIsOnWf || (bIsActive && bPierces));

            insertIsect(isect2);
            isect2->setIsWfVert(sqrdTime, bIsOnWf || (bIsActive && bPierces));
        }
    }
    
#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "After:";
    logInfo(sqrdTime);
#endif
}

bool OffCirc::deleteArc(std::pair<MovIsectPtr, MovIsectPtr> &newArc, 
                        const Root_of_2 &sqrdTime, const MovIsectPtr &from, 
                        const MovIsectPtr &to, bool bIsActive) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("deleteArc");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Before:";
    logInfo(sqrdTime);
    BOOST_LOG_SEV(slg, normal) << "Deleting arc " << from->toString()
            << " -> " << to->toString() << " ... ";
#endif
    
    bool bYieldsNewArc = false;

    if (m_arcs.size() > 2) {
        MovIsectPtr left, right;
        if (searchNeighbor(left, from, true) && searchNeighbor(right, to, false)
            && inclsArc({left->id(), from->id()}, false) 
            && inclsArc({from->id(), to->id()}, false)
            && inclsArc({to->id(), right->id()}, false)) {
            bool bIsOnWf = m_arcs.at({left->id(), from->id()});

            eraseArc(sqrdTime, left->id(), from->id());
            eraseArc(sqrdTime, from->id(), to->id());
            eraseArc(sqrdTime, to->id(), right->id());
            insertArc(sqrdTime, left->id(), right->id(), bIsOnWf);
            
            newArc = {left, right};
            bYieldsNewArc = true;
        } else if (inclsArc({from->id(), to->id()}, false)) {
#ifdef ENABLE_VIEW
            bool bIsOnWf = m_arcs.at({from->id(), to->id()});
            m_arcActions.push_back(std::make_tuple(sqrdTime, false,
                                                   std::make_pair(from->id(), to->id()),
                                                   bIsOnWf));
#endif
            m_arcs.erase(m_arcs.find({from->id(), to->id()}));
        }
    } else {
        eraseArc(sqrdTime, from->id(), to->id(), false);
        eraseArc(sqrdTime, to->id(), from->id(), false);

        if (m_arcs.empty()) {
            m_bIsActive = bIsActive;
#ifdef ENABLE_VIEW
            if (!m_bIsActive) {
                m_domSqrdTime = sqrdTime;
            }
#endif
        }
    }

    if (m_isects.find(from->id()) != m_isects.end()
        && m_isects.find(to->id()) != m_isects.end()) {
        from->setIsWfVert(sqrdTime, false);
        m_isects.erase(m_isects.find(from->id()));

        to->setIsWfVert(sqrdTime, false);
        m_isects.erase(m_isects.find(to->id()));
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "After:";
    logInfo(sqrdTime);
#endif
    
    return bYieldsNewArc;
}

bool OffCirc::deleteArcUnordered(std::pair<MovIsectPtr, MovIsectPtr> &newArc,
            const Root_of_2 &sqrdTime, const MovIsectPtr &isect1, 
            const MovIsectPtr &isect2, bool bIsActive) {
    CGAL_assertion(m_arcs.find({isect1->id(), isect2->id()}) == m_arcs.end()
                   || m_arcs.find({isect2->id(), isect1->id()}) == m_arcs.end());
    CGAL_assertion(m_arcs.find({isect1->id(), isect2->id()}) != m_arcs.end()
                   || m_arcs.find({isect2->id(), isect1->id()}) != m_arcs.end());

    if (m_arcs.find({isect1->id(), isect2->id()}) != m_arcs.end()) {
        return deleteArc(newArc, sqrdTime, isect1, isect2, bIsActive);
    } 
    
    return deleteArc(newArc, sqrdTime, isect2, isect1, bIsActive);
}

bool OffCirc::collapseArc(const Root_of_2 &sqrdTime,
                          const MovIsectPtr &from,
                          const MovIsectPtr &to) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("collapseArc");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Before:";
    logInfo(sqrdTime);
#endif

    const auto fIsAngleLess
            = [this](const MovIsectId &fromId,
                     const MovIsectId & toId) {
                CGAL_assertion(m_arcs.find({fromId, toId}) != m_arcs.end()
                               || m_arcs.find({toId, fromId}) != m_arcs.end());
                CGAL_assertion(m_arcs.find({fromId, toId}) == m_arcs.end()
                               || m_arcs.find({toId, fromId}) == m_arcs.end());

                return m_arcs.find({fromId, toId}) != m_arcs.end();
            };

    bool bIsLess = fIsAngleLess(from->id(), to->id());
    MovIsectPtr neighbor;
    bool bOk = searchNeighbor(neighbor, from, bIsLess);

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "Collapsing "
            << (bIsLess ? from->toString() : to->toString())
            << " -> " << (bIsLess ? to->toString() : from->toString()) << " ... ";
#endif
    
    CGAL_assertion(bOk);
    if (bOk) {
        const auto &oldArcId1 = std::make_pair(bIsLess ? from->id() : to->id(),
                                               bIsLess ? to->id() : from->id());

        const auto &oldArcId2 = std::make_pair(bIsLess ? neighbor->id() : from->id(),
                                               bIsLess ? from->id() : neighbor->id());

        const auto &newArcId = std::make_pair(bIsLess ? neighbor->id() : to->id(),
                                              bIsLess ? to->id() : neighbor->id());

        eraseArc(sqrdTime, oldArcId1);

        if (inclsArc(oldArcId2, false)) {
            bool bIsOnWf = m_arcs.at(oldArcId2);
            eraseArc(sqrdTime, oldArcId2);

            CGAL_assertion(m_isects.find(from->id()) != m_isects.end());
            if (m_isects.find(from->id()) != m_isects.end()) {
                m_isects.erase(m_isects.find(from->id()));
            }
            
            insertArc(sqrdTime, newArcId, bIsOnWf);
        } else {
            if (m_isects.find(from->id()) != m_isects.end()) {
                m_isects.erase(m_isects.find(from->id()));
            }
            
            if (m_isects.find(to->id()) != m_isects.end()) {
                m_isects.erase(m_isects.find(to->id()));
            }
        }
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "After:";
    logInfo(sqrdTime);
#endif
    
    return bIsLess;
}

std::pair<bool, bool> OffCirc::expandIsect(const Root_of_2 &sqrdTime,
                                           const MovIsectPtr &from,
                                           const MovIsectPtr &to,
                                           bool bIsOnWf1) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("expandIsect");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Before:";
    logInfo(sqrdTime);
#endif

    const auto fIsAngleLess
            = [this](const Root_of_2 &sqrdTime, const MovIsectPtr &isect1,
                     const MovIsectPtr &isect2) {
#ifdef ENABLE_LOGGING
                BOOST_LOG_NAMED_SCOPE("isAngleLess");
                src::severity_logger<severity_level> slg;
#endif

                const auto step = sqrdTime / static_cast<int>(1e20);
                double angle1 = m_site->angle(isect1->pntAt(sqrdTime + step)),
                        angle2 = m_site->angle(isect2->pntAt(sqrdTime + step));

                CGAL_assertion(std::abs(angle1 - angle2) < 1e-2);
                CGAL_assertion(std::abs(angle1 - angle2) > 1e-12);
                if (std::abs(angle1 - angle2) < 1e-12) {
#ifdef ENABLE_LOGGING
                    BOOST_LOG_SEV(slg, error) << "Angle difference too low!";
                    BOOST_LOG_SEV(slg, error) << "angle1 equals " << angle1
                            << " and angle2 equals " << angle2 << ".";
                    std::cout << "Angle difference too low! "
                            << angle1 << " " << angle2 << "\n";
#endif
                }

                if (std::abs(angle1 - angle2) > 1e-2) {
#ifdef ENABLE_LOGGING
                    BOOST_LOG_SEV(slg, error) << "Angle difference too high!";
                    BOOST_LOG_SEV(slg, error) << "angle1 equals " << angle1
                            << " and angle2 equals " << angle2 << ".";
                    std::cout << "Angle difference too high! "
                            << angle1 << " " << angle2 << "\n";
#endif
                }

                return angle1 < angle2;
            };
    
    bool bIsLess = fIsAngleLess(sqrdTime, from, to);
    MovIsectPtr neighbor;
    bool bOk = searchNeighbor(neighbor, from, !bIsLess);
    const auto &newArcId1 = std::make_pair(bIsLess ? from->id() : to->id(),
                                           bIsLess ? to->id() : from->id());

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "Expanding "
            << (bIsLess ? from->toString() : to->toString())
            << " -> " << (bIsLess ? to->toString() : from->toString());
#endif

    if (bOk) {
        const auto oldArcId = std::make_pair(bIsLess ? from->id() : neighbor->id(),
                                             bIsLess ? neighbor->id() : from->id());
        const auto newArcId2 = std::make_pair(bIsLess ? to->id() : neighbor->id(),
                                              bIsLess ? neighbor->id() : to->id());

        if (inclsArc(oldArcId, true)) {
            bool bIsOnWf2 = m_arcs[oldArcId];
            eraseArc(sqrdTime, oldArcId);

            insertArc(sqrdTime, newArcId1, bIsOnWf1);
            insertArc(sqrdTime, newArcId2, bIsOnWf2);
        }
    } else {
        insertArc(sqrdTime, newArcId1, bIsOnWf1);
    }
    
    insertIsect(to);

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "After:";
    logInfo(sqrdTime);
#endif
    
    return {bOk, bIsLess};
}

bool OffCirc::replaceIsect(const Root_of_2 &sqrdTime, 
                           const MovIsectPtr &oldIsect,
                           const MovIsectPtr &newIsect) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("replaceIsect");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Before:";
    logInfo(sqrdTime);
#endif

    const auto fFindArcId
            = [this](const MovIsectId & isectId) {
                CGAL_assertion(m_lefts.find(isectId) == m_lefts.end()
                               || m_rights.find(isectId) == m_rights.end());
                if (m_lefts.find(isectId) != m_lefts.end()) {
                    const auto leftId = m_lefts.at(isectId);
                    const auto arcId = std::make_pair(leftId, isectId);
                    if (inclsArc(arcId, false)) {
                        return arcId;
                    }
                }

                if (m_rights.find(isectId) != m_rights.end()) {
                    const auto rightId = m_rights.at(isectId);
                    const auto arcId = std::make_pair(isectId, rightId);
                    if (inclsArc(arcId, false)) {
                        return arcId;
                    }
                }

                CGAL_assertion(false);
                return std::make_pair(MovIsectId{}, MovIsectId{});
            };

    const auto &arcId = fFindArcId(oldIsect->id());
    bool bIsLess = oldIsect->id() == arcId.first;
    const auto otherIsectId = bIsLess ? arcId.second : arcId.first;

    CGAL_assertion(m_isects.find(otherIsectId) != m_isects.end());
    if (m_isects.find(otherIsectId) != m_isects.end()) {
        CGAL_assertion(m_isects.find(oldIsect->id()) != m_isects.end());
        if (m_isects.find(oldIsect->id()) != m_isects.end()) {
            bool bIsOnWf = oldIsect->isWfVert();
            
            m_isects.erase(oldIsect->id());
            insertIsect(newIsect);

            if (inclsArc(arcId, true)) {
                eraseArc(sqrdTime, arcId);
                insertArc(sqrdTime,
                          bIsLess ? newIsect->id() : otherIsectId,
                          bIsLess ? otherIsectId : newIsect->id(), bIsOnWf);
            }
        }
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, normal) << "After:";
    logInfo(sqrdTime);
#endif
    
    return !bIsLess;
}

bool OffCirc::searchNeighbor(MovIsectPtr &neighbor, const MovIsectPtr &isect,
                             bool bLeft) const {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("searchNeighbor");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Searching " << (bLeft ? "left" : "right")
            << " neighbor of " << isect->toString() << ".";
#endif
    
    if (bLeft && m_lefts.find(isect->id()) != m_lefts.end()) {
        const auto id = m_lefts.at(isect->id());

        CGAL_assertion(m_isects.find(id) != m_isects.end());
        if (m_isects.find(id) != m_isects.end()) {
            neighbor = m_isects.at(id);
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Found neighbor "
                    << neighbor->toString() << ".";
#endif
            return true;
        }
    }

    if (!bLeft && m_rights.find(isect->id()) != m_rights.end()) {
        const auto id = m_rights.at(isect->id());

        CGAL_assertion(m_isects.find(id) != m_isects.end());
        if (m_isects.find(id) != m_isects.end()) {
            neighbor = m_isects.at(id);
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Found neighbor "
                    << neighbor->toString() << ".";
#endif
            return true;
        }
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, warning) << "No neighbor found.";
#endif
    return false;
}

bool OffCirc::searchNeighbor(MovIsectPtr &neighbor, const Root_of_2 &sqrdTime,
                             const Circular_arc_point_2 &arcPnt, bool bLeft) const {
    double angle = m_site->angle(arcPnt);
    bool bFound = searchNeighbor(neighbor, sqrdTime, angle, bLeft);

    if (!bFound) {
        bFound = searchNeighbor(neighbor, sqrdTime, bLeft ? 2. * M_PI : 0., bLeft);
    }

    return bFound;
}

bool OffCirc::inclsIsect(const MovIsectPtr &isect) const {
    return m_isects.find(isect->id()) != m_isects.end();
}

bool OffCirc::isInActiveArc(const Root_of_2 &sqrdTime, 
                            const Circular_arc_point_2 &arcPnt) const {
    if (m_isects.empty()) {
        return m_bIsActive;
    }

    MovIsectPtr left, right;
    if (searchNeighbor(left, sqrdTime, arcPnt, true)
        && searchNeighbor(right, sqrdTime, arcPnt, false)) {
        return inclsArc({left->id(), right->id()}, false);
    }

    return false;
}

bool OffCirc::searchNeighbor(MovIsectPtr &neighbor, const Root_of_2 &sqrdTime,
                             double angle1, bool bLeft) const {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("searchNeighbor");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Searching " << (bLeft ? "left" : "right")
            << " neighbor of isect with angle " << angle1 << ".";
#endif
    
    double minDiff = std::numeric_limits<double>::max();
    bool bFound = false;

    for (const auto &val : m_isects) {
        const auto isect = val.second;
        double angle2 = m_site->angle(isect->pntAt(sqrdTime));

        if (((bLeft && angle2 < angle1) || (!bLeft && angle2 > angle1))
            && CGAL::abs(angle1 - angle2) < minDiff) {
            minDiff = std::abs(angle1 - angle2);
            bFound = true;
            neighbor = isect;
        }
    }

#ifdef ENABLE_LOGGING
    if (bFound) {
        BOOST_LOG_SEV(slg, normal) << "Found neighbor " << neighbor->toString() << ".";
    } else {
        BOOST_LOG_SEV(slg, warning) << "No neighbor found.";
    }
#endif

    return bFound;
}

#ifdef ENABLE_LOGGING

void OffCirc::logInfo(const Root_of_2 &sqrdTime) const {
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "\tArcs of offset circle " << site()->id() << ":";
    
    for (const auto &val : m_arcs) {
        const auto arc = val.first;
        BOOST_LOG_SEV(slg, normal) << "\t(" 
                << std::get<0>(arc.first) << ", "
                << std::get<1>(arc.first) << ", "
                << std::get<2>(arc.first) << ", "
                << std::get<3>(arc.first) << ") -> ("
                << std::get<0>(arc.second) << ", "
                << std::get<1>(arc.second) << ", "
                << std::get<2>(arc.second) << ", "
                << std::get<3>(arc.second) << ")";
    }
    
    BOOST_LOG_SEV(slg, normal) << "\tIsects of offset circle " << site()->id();
    for (const auto &val : m_isects) {
        const auto isect = val.second;
        BOOST_LOG_SEV(slg, normal) << "\t" << isect->toString() << " with angle "
                << m_site->angle(isect->pntAt(sqrdTime)) << ".";
    }
    
    BOOST_LOG_SEV(slg, normal) << "\tArcs of offset circle " << site()->id() << ":";
}

#endif

void OffCirc::insertArc(const Root_of_2 &sqrdTime,
                        const std::pair<MovIsectId, MovIsectId> &arcId,
                        bool bIsOnWf) {
    insertArc(sqrdTime, arcId.first, arcId.second, bIsOnWf);
}

void OffCirc::insertArc(const Root_of_2 &
#ifdef ENABLE_VIEW
                        sqrdTime
#endif
                        , const MovIsectId &fromId,
                        const MovIsectId &toId, bool bIsOnWf) {
    m_arcs[{fromId, toId}] = bIsOnWf;
    m_rights[fromId] = toId;
    m_lefts[toId] = fromId;
#ifdef ENABLE_VIEW
    m_arcActions.push_back(std::make_tuple(sqrdTime, true, 
                                           std::make_pair(fromId, toId),
                                           bIsOnWf));
#endif
}

void OffCirc::eraseArc(const Root_of_2 &sqrdTime, 
                       const std::pair<MovIsectId, MovIsectId> &arcId,
                       bool bAssert) {
    eraseArc(sqrdTime, arcId.first, arcId.second, bAssert);
}

void OffCirc::eraseArc(const Root_of_2 &
#ifdef ENABLE_VIEW
                       sqrdTime
#endif
                       , const MovIsectId &fromId, const MovIsectId &toId,
                       bool bAssert) {
    const auto &arcId = std::make_pair(fromId, toId);

    if (inclsArc(arcId, bAssert)) {
#ifdef ENABLE_VIEW
        bool bIsOnWf = m_arcs.at(arcId);
        m_arcActions.push_back(std::make_tuple(sqrdTime, false,
                                               std::make_pair(fromId, toId),
                                               bIsOnWf));
#endif
        m_arcs.erase(arcId);
    }
    
    if (bAssert) {
        CGAL_assertion(m_rights.find(fromId) != m_rights.end());
    }
    
    if (m_rights.find(fromId) != m_rights.end()) {
        m_rights.erase(fromId);
    }
    
    if (bAssert) {
        CGAL_assertion(m_lefts.find(toId) != m_lefts.end());
    }
    
    if (m_lefts.find(toId) != m_lefts.end()) {
        m_lefts.erase(toId);
    }
}

void OffCirc::insertIsect(const MovIsectPtr &isect) {
    m_isects[isect->id()] = isect;
#ifdef ENABLE_VIEW
    m_isectHistory[isect->id()] = isect;
#endif
}

bool OffCirc::inclsArc(const std::pair<MovIsectId, MovIsectId> &arcId,
                       bool bAssert) const {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("inclsArc");
    src::severity_logger<severity_level> slg;
#endif
    
    if (bAssert) {
        CGAL_assertion(m_arcs.find(arcId) != m_arcs.end());
#ifdef ENABLE_LOGGING
        if (m_arcs.find(arcId) == m_arcs.end()) {
            BOOST_LOG_SEV(slg, error) << "Offset circle does not include arc ("
                    << std::get<0>(arcId.first) << ", "
                    << std::get<1>(arcId.first) << ", "
                    << std::get<2>(arcId.first) << ", "
                    << std::get<3>(arcId.first) << ") -> ("
                    << std::get<0>(arcId.second) << ", "
                    << std::get<1>(arcId.second) << ", "
                    << std::get<2>(arcId.second) << ", "
                    << std::get<3>(arcId.second) << ")";
        }
#endif
    }

    return m_arcs.find(arcId) != m_arcs.end();
}

#ifdef ENABLE_VIEW

OffCircGraphicsItem::OffCircGraphicsItem(const OffCircPtr &offCirc)
: m_offCirc{offCirc}
, m_center{CGAL::to_double(offCirc->site()->center().x()),
        CGAL::to_double(offCirc->site()->center().y())}
{
    for (const auto &isect : m_offCirc->isectHistory()) {
        m_switches[isect.first] = 0;
    }
}

QRectF OffCircGraphicsItem::boundingRect() const {
    return m_boundingRect;
}

void OffCircGraphicsItem::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *, QWidget *) {
    const auto ih = m_offCirc->isectHistory();

    if (m_arcs.empty()
        && (m_sqrdTime < m_offCirc->domSqrdTime()
            || m_offCirc->domSqrdTime() < 0.)) {
        if (typeid(*m_offCirc->site()) == typeid(PntSite)) {
            const auto pntSite
                    = std::static_pointer_cast<PntSite>(m_offCirc->site());
            const auto circ = pntSite->growOffAt(m_sqrdTime);
            painter->setPen(QPen{QBrush{Qt::blue}, 2.});
            Util::draw(painter, circ);
        }
        
        return;
    }
    
    for (const auto &arc : m_arcs) {
        const auto arcId = arc.first;
        bool bIsOnWf = arc.second;
        CGAL_assertion(ih.find(arcId.first) != ih.end()
                       && ih.find(arcId.second) != ih.end());
        const auto isect1 = ih.at(arcId.first), isect2 = ih.at(arcId.second);
        const auto &pnt1 = isect1->pntAt(m_sqrdTime),
                &pnt2 = isect2->pntAt(m_sqrdTime);
        bool bIsWfVert1 = false, bIsWfVert2 = false;

        if (!m_switches.empty()) {
            size_t si1 = m_switches.at(isect1->id()),
                    si2 = m_switches.at(isect2->id());
            const auto sw1 = isect1->switches(), sw2 = isect2->switches();

            if (si1 > 0) {
                bIsWfVert1 = sw1.at(si1 - 1).second;
            }

            if (si2 > 0) {
                bIsWfVert2 = sw2.at(si2 - 1).second;
            }
        }

        if (typeid(*m_offCirc->site()) == typeid(PntSite)) {
            const auto pntSite
                    = std::static_pointer_cast<PntSite>(m_offCirc->site());
            const auto circ = pntSite->growOffAt(m_sqrdTime);
            const auto arc = Circular_arc_2{circ, pnt1, pnt2};
            painter->setPen(QPen{QBrush{bIsOnWf ? Qt::blue : Qt::lightGray}, 2.});
            Util::draw(painter, arc);
        }
        
        auto brush = QBrush{bIsWfVert1 ? Qt::blue : Qt::lightGray};
        painter->setBrush(brush);
        painter->setPen(QPen{brush, 2.});
        Util::draw(painter, pnt1);
        
        brush = QBrush{bIsWfVert2 ? Qt::blue : Qt::lightGray};
        painter->setBrush(brush);
        painter->setPen(QPen{brush, 2.});
        Util::draw(painter, pnt2);
    }
}
    
void OffCircGraphicsItem::modelChanged() {
    update();
}

void OffCircGraphicsItem::onTimeChanged(double t) {
    const auto sqrdTime = Root_of_2{t};
    const auto actions = m_offCirc->arcActions();
    const auto ih = m_offCirc->isectHistory();
    
    if (sqrdTime <= 0.) {
        m_boundingRect = QRect{};
        update();
        return;
    }
    
    if (m_sqrdTime < sqrdTime && m_step < actions.size()) {
        m_sqrdTime = sqrdTime;

        auto nextAction = actions.at(m_step);
        auto nextSqrdTime = std::get<0>(nextAction);
        bool bFinished = false;
            
        while (nextSqrdTime < m_sqrdTime && !bFinished) {
            bool bInsert = std::get<1>(nextAction);
            const auto arcId = std::get<2>(nextAction);
            const auto isect1 = ih.at(arcId.first), isect2 = ih.at(arcId.second);
            size_t si1 = m_switches.at(isect1->id()),
                    si2 = m_switches.at(isect2->id());
            
            if (si1 < isect1->switches().size() - 1) {
                const auto nsw = isect1->switches().at(si1);
                if (nsw.first < m_sqrdTime) {
                    m_switches[isect1->id()] = si1 + 1;
                }
            }
            
            if (si2 < isect2->switches().size() - 1) {
                const auto nsw = isect2->switches().at(si2);
                if (nsw.first < m_sqrdTime) {
                    m_switches[isect2->id()] = si2 + 1;
                }
            }
            
            if (bInsert) {
                m_arcs[arcId] = std::get<3>(nextAction);
            } else if (m_arcs.find(arcId) != m_arcs.end()) {
                m_arcs.erase(m_arcs.find(arcId));
            }

            if (m_step + 1 < actions.size()) {
                nextAction = actions.at(++m_step);
                nextSqrdTime = std::get<0>(nextAction);
            } else {
                ++m_step;
                bFinished = true;
            }
        }
    } else if (m_sqrdTime > sqrdTime && 0 < m_step) {
        m_sqrdTime = sqrdTime;

        auto prevAction = actions.at(m_step - 1);
        auto prevSqrdTime = std::get<0>(prevAction);
        bool bFinished = false;

        while (prevSqrdTime > m_sqrdTime && !bFinished) {
            bool bInsert = std::get<1>(prevAction);
            const auto arcId = std::get<2>(prevAction);
            const auto isect1 = ih.at(arcId.first), isect2 = ih.at(arcId.second);
            int si1 = m_switches.at(isect1->id()),
                    si2 = m_switches.at(isect2->id());
            
            if (si1 > 0) {
                const auto psw = isect1->switches().at(si1 - 1);
                if (psw.first > m_sqrdTime) {
                    m_switches[isect1->id()] = si1 - 1;
                }
            }
            
            if (si2 > 0) {
                const auto psw = isect2->switches().at(si2 - 1);
                if (psw.first > m_sqrdTime) {
                    m_switches[isect2->id()] = si2 - 1;
                }
            }

            if (!bInsert) {
                m_arcs[arcId] = std::get<3>(prevAction);
            } else if (m_arcs.find(arcId) != m_arcs.end()) {
                m_arcs.erase(m_arcs.find(arcId));
            }
            
            if (m_step - 1 > 0) {
                prevAction = actions.at((--m_step) - 1);
                prevSqrdTime = std::get<0>(prevAction);
            } else {
                --m_step;
                bFinished = true;
            }
        }
    }
    
    updateBoundingRect();
    update();
}

void OffCircGraphicsItem::updateBoundingRect() {
    if (typeid(*m_offCirc->site()) == typeid(PntSite)) {
        const auto pntSite
                = std::static_pointer_cast<PntSite>(m_offCirc->site());
        const auto circ = pntSite->growOffAt(m_sqrdTime);
        m_boundingRect = Util::boundingRect(circ);
    }
}

#include "moc_offcirc.cpp"

#endif
