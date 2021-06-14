#include <iostream>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <CGAL/MP_Float.h>
#include "file_io.h"
#include "offcirc.h"
#include "overlay.h"
#include "mwvd.h"
#include "util.h"
#include "log.h"

Ev::Ev(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
       const SitePtr &site)
: m_arcPnt{arcPnt}
, m_sqrdTime{sqrdTime}
, m_site{site}
{
}

CollEv::CollEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
               const SitePtr &site, const TrajPtr &traj1, const TrajPtr &traj2, 
               bool bPierces)
: Ev{arcPnt, sqrdTime, site}
, m_traj1{traj1}
, m_traj2{traj2}
, m_bPierces{bPierces}
{
}

EdgeEv::EdgeEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
               const SitePtr &site, const MovIsectPtr &isect1, 
               const MovIsectPtr &isect2)
: Ev{arcPnt, sqrdTime, site}
, m_isect1{isect1}
, m_isect2{isect2}
{
}

DomEv::DomEv(const Circular_arc_point_2 &arcPnt, const Root_of_2 &sqrdTime, 
             const SitePtr &site, const MovIsectPtr &isect1, 
             const MovIsectPtr &isect2)
: Ev{arcPnt, sqrdTime, site}
, m_isect1{isect1}
, m_isect2{isect2}
{
}

VorDiag::VorDiag(const std::string &inFilePath, bool bUseOverlay,
                 const std::string &ipeFilePath, const std::string &csvFilePath,
                 bool bEnableView) {
    const auto fr = FileReader{inFilePath};
    
    for (const auto &s : fr.sites()) {
        int x = std::get<0>(s), y = std::get<1>(s), w = std::get<2>(s),
                siteIndex = std::get<3>(s);
        m_sites.push_back(std::make_shared<PntSite>(Point_2{x, y}, w, siteIndex));
    }

    std::sort(m_sites.begin(), m_sites.end(),
              [](const SitePtr &lhs, const SitePtr & rhs) {
                  return lhs->weight() > rhs->weight();
              });
    
    size_t n = m_sites.size(), maxCandSetSize = 0, nCandSets = 0, avgSize = 0;

    for (const auto &site : m_sites) {
        m_offCircs[site->id()] = std::make_shared<OffCirc>(site);
    }

    std::cout << "Computing collisions ...\n";
    std::chrono::high_resolution_clock::time_point t0;
    long long overlayDuration = 0;

    std::cout << "Inserting initial collision events into event queue ...\n";
    if (!bUseOverlay) {
        t0 = std::chrono::high_resolution_clock::now();
        compColls();
    } else {
        const auto overlay = Overlay(inFilePath);
        overlayDuration = overlay.duration();
        nCandSets = overlay.candSets().size();
        maxCandSetSize = overlay.maxSize();
        avgSize = overlay.avgSize();

        t0 = std::chrono::high_resolution_clock::now();
        compColls(overlay.candSets());
    }

    std::cout << "Processing events ...\n";
    std::chrono::high_resolution_clock::time_point t1 =
            std::chrono::high_resolution_clock::now();
    run();
    std::chrono::high_resolution_clock::time_point t2 =
            std::chrono::high_resolution_clock::now();

    if (bEnableView || !ipeFilePath.empty()) {
        std::cout << "Computing Voronoi edges ...\n";
        compVorEdges();
    }

    if (!ipeFilePath.empty()) {
        std::cout << "Writing output to " << ipeFilePath << " ...\n";
        writeToIpe(ipeFilePath);
    }

    long long duration1 =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    long long duration2 =
            std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    long long runtime = duration1 + duration2;

    if (bUseOverlay) {
        duration1 += overlayDuration;
        runtime += overlayDuration;
    }

    if (!csvFilePath.empty()) {
        std::cout << "Writing output to" << csvFilePath << " ...\n";
        writeToCsv(csvFilePath, runtime, duration1, duration2, maxCandSetSize, nCandSets, avgSize);
    }

    long long normRuntime = runtime / (n * std::pow(std::log2(n), 2));
    std::cout << "Run successful!\n"
            << "Computing the initial collisions took " << duration1 << " microseconds.\n"
            << "Processing the events took " << duration2 << " microseconds.\n"
            << "The overall runtime equals " << runtime << " microseconds.\n"
            << "The runtime divided through n log(n)^2 equals " << normRuntime << " microseconds.\n"
            << "Overall\n"
            << "\t" << m_collEvCnt << " valid collision events,\n"
            << "\t" << m_domEvCnt << " valid domination events, and\n"
            << "\t" << m_edgeEvCnt << " valid edge events took place.\n"
            << "Additionally,\n"
            << "\t" << m_invalidCollEvCnt << " invalid collision events,\n"
            << "\t" << m_invalidDomEvCnt << " invalid domination events, and\n"
            << "\t" << m_invalidEdgeEvCnt << " invalid edge events were ignored.\n"
            << "\t" << m_nVorVerts << " Voronoi nodes have been found.\n";
}

void VorDiag::compColls(const std::vector<std::set<int>> &candSets) {
    std::set<std::pair<int, int>> candColls;
    for (const auto &candSet : candSets) {
        for (int cand1 : candSet) {
            for (int cand2 : candSet) {
                if (cand1 != cand2 && cand1 != -1 && cand2 != -1) {
                    const auto site1 = m_sites.at(cand1), site2 = m_sites.at(cand2);
                    const auto candPair = std::minmax({cand1, cand2});
                    
                    if (candColls.find(candPair) == candColls.end()) {
                        candColls.insert(candPair);

                        const auto id = std::minmax({site1->id(), site2->id()});

                        if (m_bisecs.find(id) == m_bisecs.end()) {
                            const auto bisec
                                    = BisecFactory{site1, site2}.makeBisec();
                            m_bisecs[id] = bisec;
                        }

                        if (m_bisecs.at(id)->trajs().size() == 2) {
                            const auto traj1 = m_bisecs.at(id)->trajs().at(0),
                                    traj2 = m_bisecs.at(id)->trajs().at(1);

                            m_queue.push(std::make_shared<CollEv>(traj1->start()->arcPnt(),
                                                                  traj1->start()->sqrdTime(),
                                                                  site1, traj1,
                                                                  traj2, false));
                            m_queue.push(std::make_shared<CollEv>(traj1->start()->arcPnt(),
                                                                  traj1->start()->sqrdTime(),
                                                                  site2, traj1,
                                                                  traj2, false));
                        } else if (m_bisecs.at(id)->trajs().size() == 4) {
                            CGAL_assertion(false);
                        }
                    }
                }
            }
        }
    }
}

void VorDiag::compColls() {
    for (const auto &val1 : m_offCircs) {
        for (const auto &val2 : m_offCircs) {
            const auto site1 = val1.second->site(), site2 = val2.second->site();
            compColl(site1, site2);
        }
    }
}

void VorDiag::compColl(const SitePtr &site1, const SitePtr &site2) {
    if (site1->id() != site2->id()) {
        const auto id = std::minmax({site1->id(), site2->id()});

        if (m_bisecs.find(id) == m_bisecs.end()) {
            m_bisecs[id] = BisecFactory(site1, site2).makeBisec();
        }

        CGAL_assertion(m_bisecs.at(id)->trajs().size() == 2
               || m_bisecs.at(id)->trajs().size() == 4);

        if (m_bisecs.at(id)->trajs().size() == 2) {
            const auto traj1 = m_bisecs[id]->trajs().at(0),
                    traj2 = m_bisecs[id]->trajs().at(1);

            m_queue.push(std::make_shared<CollEv>(traj1->start()->arcPnt(),
                                                  traj1->start()->sqrdTime(),
                                                  site1, traj1, traj2, false));

            const auto isect1 = makeMovIsect(traj1), isect2 = makeMovIsect(traj2);
            m_queue.push(std::make_shared<DomEv>(isect1->traj()->end()->arcPnt(),
                                                 isect1->traj()->end()->sqrdTime(),
                                                 site1, isect1, isect2));
        }
    }
}

void VorDiag::run() {
    while (!m_queue.empty()) {
        const auto ev = m_queue.top();
        m_queue.pop();

        switch (ev->type()) {
            case EvType::Coll:
            {
                const auto collEv = std::static_pointer_cast<CollEv>(ev);
                handleEv(collEv);
                break;
            }
            case EvType::Edge:
            {
                const auto edgeEv = std::static_pointer_cast<EdgeEv>(ev);
                handleEv(edgeEv);
                break;
            }
            case EvType::Dom:
            {
                const auto domEv = std::static_pointer_cast<DomEv>(ev);
                handleEv(domEv);
                break;
            }
            case EvType::None:
            default:
                break;
        }
    }
}

void VorDiag::handleEv(const std::shared_ptr<CollEv> &collEv1) {
    const auto ev = m_queue.top();

    CGAL_assertion(ev->type() == EvType::Coll);
    if (ev->type() == EvType::Coll) {
        m_queue.pop();
        const auto collEv2 = std::static_pointer_cast<CollEv>(ev);
        bool bDom = (*collEv1->site() > *collEv2->site()),
                bIsValid = isValidCollEv(collEv1, collEv2);
        
        CGAL_assertion(collEv1->sqrdTime() == collEv2->sqrdTime());
        CGAL_assertion(collEv1->pierces() == collEv2->pierces());
        processEv(collEv1, bDom, bIsValid, collEv1->pierces());
        processEv(collEv2, !bDom, bIsValid, collEv1->pierces());
    }
}

bool VorDiag::isValidCollEv(const std::shared_ptr<CollEv> &collEv1,
                            const std::shared_ptr<CollEv> &collEv2) const {
    const auto offCirc1 = m_offCircs.at(collEv1->site()->id()),
            offCirc2 = m_offCircs.at(collEv2->site()->id());
    const auto arcPnt = collEv1->arcPnt();
    const auto sqrdTime = collEv1->sqrdTime();

    return offCirc1->isInActiveArc(sqrdTime, arcPnt) && offCirc2->isInActiveArc(sqrdTime, arcPnt);
}

bool VorDiag::isValidDomEv(const std::shared_ptr<DomEv> &domEv1,
                           const std::shared_ptr<DomEv> &domEv2) const {
    const auto offCirc1 = m_offCircs.at(domEv1->site()->id()),
            offCirc2 = m_offCircs.at(domEv2->site()->id());
    const auto arcPnt = domEv1->arcPnt();
    const auto sqrdTime = domEv1->sqrdTime();

    return (offCirc1->isInActiveArc(sqrdTime, arcPnt) && offCirc2->isInActiveArc(sqrdTime, arcPnt))
            || (offCirc1->isActive() && offCirc2->isActive());
}

void VorDiag::processEv(const std::shared_ptr<CollEv> &collEv, bool bDom,
                        bool bIsValid, bool bPierces) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("processCollEv");
    src::severity_logger<severity_level> slg;
#endif
    
    const auto sqrdTime = collEv->sqrdTime();
    const auto site = collEv->site();
    const auto offCirc = m_offCircs.at(site->id());
    const auto &isect1 = makeMovIsect(collEv->traj1()),
            &isect2 = makeMovIsect(collEv->traj2());
    
    CGAL_assertion(collEv->traj1()->isLeft() && !collEv->traj2()->isLeft());

    if (bIsValid) {
#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, normal) << "Triggered valid collision event at time "
                << std::sqrt(CGAL::to_double(sqrdTime))
                << " along offset circle " << collEv->site()->id() << ".";
#endif
    
        offCirc->spawnArc(sqrdTime, isect1, isect2, bDom, bPierces);
        checkEdgeEv(offCirc, sqrdTime, isect1, bPierces ? !bDom : bDom);
        checkEdgeEv(offCirc, sqrdTime, isect2, bPierces ? bDom : !bDom);
        storeEv(collEv);
    } else {
#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, normal) << "Ignored invalid collision event at time "
                << std::sqrt(CGAL::to_double(sqrdTime))
                << " along offset circle " << collEv->site()->id() << ".";
#endif
        m_invalidCollEvCnt++;
    }

    m_queue.push(std::make_shared<DomEv>(isect1->traj()->end()->arcPnt(),
                                         isect1->traj()->end()->sqrdTime(),
                                         site, isect1, isect2));
}

void VorDiag::handleEv(const std::shared_ptr<DomEv> &domEv1) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("handleDomEv");
    src::severity_logger<severity_level> slg;
#endif
    
    const auto ev = m_queue.top();
    m_queue.pop();
    
    CGAL_assertion(ev->type() == EvType::Dom);
    if (ev->type() == EvType::Dom) {
        const auto domEv2 = std::static_pointer_cast<DomEv>(ev);
        //bool bDom = domEv1->site()->weight() > domEv2->site()->weight();
        bool bDom = *domEv1->site() > *domEv2->site();
    
        CGAL_assertion(domEv1->sqrdTime() == domEv2->sqrdTime());

        if (isValidDomEv(domEv1, domEv2)) {
            processEv(domEv1, bDom);
            processEv(domEv2, !bDom);
        } else {
            m_invalidDomEvCnt += 2;
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal)
                    << "Ignored two invalid domination events at time "
                    << std::sqrt(CGAL::to_double(domEv1->sqrdTime()))
                    << " along the offset circles "
                    << domEv1->site()->id() << " and "
                    << domEv2->site()->id() << ".";
#endif
        }
    }
}

void VorDiag::processEv(const std::shared_ptr<DomEv> &domEv, bool bDom) {
    const auto sqrdTime = domEv->sqrdTime();
    const auto site = domEv->site();
    const auto offCirc = m_offCircs.at(site->id());
    const auto isect1 = domEv->isect1(), isect2 = domEv->isect2();

#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("processDomEv");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Triggered valid domination event at time "
            << std::sqrt(CGAL::to_double(sqrdTime))
            << " along offset circle " << domEv->site()->id() << ".";
#endif
    
    std::pair<MovIsectPtr, MovIsectPtr> newArc;
    bool bOk = offCirc->deleteArc(newArc, sqrdTime, isect1, isect2, bDom);
    if (bOk) {
        checkEdgeEv(offCirc, sqrdTime, newArc.first, newArc.second);
    }

    storeEv(domEv);
}

void VorDiag::handleEv(const std::shared_ptr<EdgeEv> &edgeEv) {
    const auto sqrdTime = edgeEv->sqrdTime();
    const auto site1 = edgeEv->site(),
            &site2 = otherSite(site1, edgeEv->isect1()),
            &site3 = otherSite(site1, edgeEv->isect2());

#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("handleEdgeEv");
    src::severity_logger<severity_level> slg;
    BOOST_LOG_SEV(slg, normal) << "Triggered edge event at time "
            << std::sqrt(CGAL::to_double(sqrdTime))
            << " involving the sites "
            << site1->id() << " (with weight " << site1->weight() << "), "
            << site2->id() << " (with weight " << site2->weight() << "), "
            << site3->id() << " (with weight " << site3->weight() << ").";
#endif

    std::set<int> ids;
    ids.insert(edgeEv->site()->id());
    std::vector<std::shared_ptr<EdgeEv>> twins;
    auto nextEv = m_queue.top();

#ifdef ENABLE_LOGGING
    if (edgeEv->arcPnt() == nextEv->arcPnt()) {
        BOOST_LOG_SEV(slg, normal) << "The current event point equals the next event point.";
    }
#endif
    while (typeid(*nextEv) == typeid(EdgeEv) && edgeEv->arcPnt() == nextEv->arcPnt()) {
        const auto twin = std::static_pointer_cast<EdgeEv>(nextEv);
        const auto offCirc = m_offCircs.at(nextEv->site()->id());
        const auto isect1 = twin->isect1(), isect2 = twin->isect2();

        if (ids.find(nextEv->site()->id()) == ids.end()
            && offCirc->inclsIsect(isect1) && offCirc->inclsIsect(isect2)) {
            twins.push_back(twin);
            ids.insert(nextEv->site()->id());

#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Found twin event at site "
                    << nextEv->site()->id() << ".";
#endif
            m_edgeEvCnt++;
        } else {
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, warning) << "Invalid edge event found!";
#endif
            m_invalidEdgeEvCnt++;
        }
        
        m_queue.pop();
        nextEv = m_queue.top();
    }

    if (!twins.empty()) {
#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, normal) << "Three arcs vanished simultaneously.";
#endif
        SitePtr lowSite, medSite, highSite;
        if (twins.size() != 2) {
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, warning) << "Number of twin events does not equal two!";
#endif
            std::vector<SitePtr> sites{site1, site2, site3};
            std::sort(sites.begin(), sites.end(),
                      [](const SitePtr &lhs, const SitePtr & rhs) {
                          //return lhs->weight() < rhs->weight();
                          return *lhs < *rhs;
                      }
                      );

            lowSite = sites.at(0);
            medSite = sites.at(1);
            highSite = sites.at(2);
        } else {
            twins.push_back(edgeEv);
            std::sort(twins.begin(), twins.end(),
                      [](const std::shared_ptr<EdgeEv> &lhs,
                         const std::shared_ptr<EdgeEv> & rhs) {
                          //return lhs->site()->weight() < rhs->site()->weight();
                          return *lhs->site() < *rhs->site();
                      }
                      );

            lowSite = twins.at(0)->site();
            medSite = twins.at(1)->site();
            highSite = twins.at(2)->site();
        }

        if (!deleteLowestArc(edgeEv, lowSite, medSite, highSite)) {
            return;
        }
    } else {
        if (*site1 > *site2 && *site1 > *site3) {
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Arc vanished along the highest weighted site.";
#endif
            const auto lowSite = *site2 < *site3 ? site2 : site3,
                    medSite = *site2 < *site3 ? site3 : site2;

            const auto &isect1 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, medSite),
                    &isect2 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, site1),
                    &isect3 = makeMovIsectAt(edgeEv->arcPnt(), medSite, site1);

            if (!isect1 || !isect2 || !isect3) {
                return;
            }
            
            const auto low = m_offCircs.at(lowSite->id()),
                    med = m_offCircs.at(medSite->id()),
                    high = m_offCircs.at(site1->id());

            if (!low->inclsIsect(isect2) || !med->inclsIsect(isect3)
                || !high->inclsIsect(isect2) || !high->inclsIsect(isect3)) {
#ifdef ENABLE_LOGGING
                BOOST_LOG_SEV(slg, warning) << "Invalid edge event found!";
#endif
                m_invalidEdgeEvCnt++;
                return;
            }

            bool bIsWfVert2 = isect2->isWfVert(),
                    bIsWfVert3 = isect3->isWfVert();
            const auto bCheckNeighbors = med->expandIsect(sqrdTime, edgeEv->arcPnt(),
                                                          isect3, isect1,
                                                          bIsWfVert2 && !bIsWfVert3);
            bool bLeft1 = high->collapseArc(sqrdTime, isect2, isect3),
                    bLeft3 = low->replaceIsect(sqrdTime, isect2, isect1);

            checkEdgeEv(high, sqrdTime, isect3, bLeft1);

            if (bCheckNeighbors.first) {
                checkEdgeEv(med, sqrdTime, isect1, true);
                checkEdgeEv(med, sqrdTime, isect1, false);
            } else {
                checkEdgeEv(med, sqrdTime, isect1, bCheckNeighbors.second);
            }

            checkEdgeEv(low, sqrdTime, isect1, bLeft3);

            if (bIsWfVert2 && bIsWfVert3) {
                isect1->setIsWfVert(sqrdTime, true);
                isect2->setIsWfVert(sqrdTime, false);
                isect3->setIsWfVert(sqrdTime, false);
                m_nVorVerts++;
            } else if (bIsWfVert2 && !bIsWfVert3) {
                isect1->setIsWfVert(sqrdTime, true);
                isect2->setIsWfVert(sqrdTime, false);
                isect3->setIsWfVert(sqrdTime, true);
                m_nVorVerts++;
            } else if (!bIsWfVert2 && !bIsWfVert3) {
                isect1->setIsWfVert(sqrdTime, false);
                //isect2->setIsWfVert(sqrdTime, false);
                //isect3->setIsWfVert(sqrdTime, false);
            }
        } else if (*site1 < *site2 && *site1 < *site3) {
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Arc vanished along the lowest weighted site.";
#endif
            /*std::vector<SitePtr> sites{site1, site2, site3};
            std::sort(
                      sites.begin(),
                      sites.end(),
                      [](const SitePtr &lhs, const SitePtr & rhs) {
                          return lhs->weight() < rhs->weight();
                      }
                      );

            const auto lowSite = sites.at(0), medSite = sites.at(1),
                    highSite = sites.at(2);

            if (!deleteLowestArc(edgeEv, lowSite, medSite, highSite)) {
                return;
            }*/
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, error) << "twins.size() != 2 at time " 
                    << std::sqrt(CGAL::to_double(sqrdTime)) << "!\n";
            BOOST_LOG_SEV(slg, error) << "Lowest edge vanished without twin event!\n";
#ifndef NDEBUG
            CGAL_assertion(false);
#endif
#endif
        } else { // The edge that is associated with the medium weight vanished.
#ifdef ENABLE_LOGGING
            BOOST_LOG_SEV(slg, normal) << "Arc vanished along the medium weighted site.";
#endif
            const auto lowSite = *site2 < *site3 ? site2 : site3,
                    highSite = *site2 < *site3 ? site3 : site2;
            
            const auto &isect1 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, site1),
                    &isect2 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, highSite),
                    &isect3 = makeMovIsectAt(edgeEv->arcPnt(), site1, highSite);

            if (!isect1 || !isect2 || !isect3) {
                return;
            }
            
            const auto low = m_offCircs.at(lowSite->id()),
                    med = m_offCircs.at(site1->id()),
                    high = m_offCircs.at(highSite->id());

            if (!low->inclsIsect(isect1) || !med->inclsIsect(isect1)
                || !med->inclsIsect(isect3) || !high->inclsIsect(isect3)) {
#ifdef ENABLE_LOGGING
                BOOST_LOG_SEV(slg, warning) << "Invalid edge event found!";
#endif
                m_invalidEdgeEvCnt++;
                return;
            }

            bool bIsWfVert1 = isect1->isWfVert(), bIsWfVert3 = isect3->isWfVert();
            const auto &bCheckNeighbors = high->expandIsect(sqrdTime, edgeEv->arcPnt(),
                                                            isect3, isect2,
                                                            bIsWfVert1 && !bIsWfVert3);
            bool bLeft2 = med->collapseArc(sqrdTime, isect1, isect3),
                    bLeft3 = low->replaceIsect(sqrdTime, isect1, isect2);

            if (bCheckNeighbors.first) {
                checkEdgeEv(high, sqrdTime, isect2, true);
                checkEdgeEv(high, sqrdTime, isect2, false);
            } else {
                checkEdgeEv(high, sqrdTime, isect2, bCheckNeighbors.second);
            }

            checkEdgeEv(med, sqrdTime, isect3, bLeft2);
            checkEdgeEv(low, sqrdTime, isect2, bLeft3);

            if (bIsWfVert1 && bIsWfVert3) {
                isect1->setIsWfVert(sqrdTime, false);
                isect2->setIsWfVert(sqrdTime, true);
                isect3->setIsWfVert(sqrdTime, false);
                m_nVorVerts++;
            } else if (bIsWfVert1 && !bIsWfVert3) {
                isect1->setIsWfVert(sqrdTime, false);
                isect2->setIsWfVert(sqrdTime, true);
                isect3->setIsWfVert(sqrdTime, true);
                m_nVorVerts++;
            } else if (!bIsWfVert1 && !bIsWfVert3) {
                //isect1->setIsWfVert(sqrdTime, false);
                isect2->setIsWfVert(sqrdTime, false);
                //isect3->setIsWfVert(sqrdTime, false);
            }
        }
    }

    storeEv(edgeEv);
}

bool VorDiag::deleteLowestArc(const std::shared_ptr<EdgeEv> &edgeEv,
                              const SitePtr &lowSite, const SitePtr &medSite,
                              const SitePtr &highSite) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("deleteLowestArc");
    src::severity_logger<severity_level> slg;
#endif
    const auto sqrdTime = edgeEv->sqrdTime();
    const auto &isect1 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, medSite),
            &isect2 = makeMovIsectAt(edgeEv->arcPnt(), lowSite, highSite),
            &isect3 = makeMovIsectAt(edgeEv->arcPnt(), medSite, highSite);

    if (!isect1 || !isect2 || !isect3) {
        return false;
    }
    
    const auto low = m_offCircs.at(lowSite->id()),
            med = m_offCircs.at(medSite->id()),
            high = m_offCircs.at(highSite->id());

    if (!low->inclsIsect(isect1) || !low->inclsIsect(isect2)
        || !med->inclsIsect(isect1) || !med->inclsIsect(isect3)
        || !high->inclsIsect(isect2) || !high->inclsIsect(isect3)) {
#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, warning) << "Invalid edge event found!";
#endif
        m_invalidEdgeEvCnt++;
        return false;
    }

    bool bIsWfVert1 = isect1->isWfVert(), bIsWfVert2 = isect2->isWfVert(),
            bIsWfVert3 = isect3->isWfVert();
    bool bLeft1 = high->collapseArc(sqrdTime, isect2, isect3),
            bLeft2 = med->collapseArc(sqrdTime, isect1, isect3);
    std::pair<MovIsectPtr, MovIsectPtr> newArc;
    low->deleteArcUnordered(newArc, sqrdTime, isect1, isect2, false);

    checkEdgeEv(high, sqrdTime, isect3, bLeft1);
    checkEdgeEv(med, sqrdTime, isect3, bLeft2);

    if (bIsWfVert1 && bIsWfVert2 && bIsWfVert3) {
        isect1->setIsWfVert(sqrdTime, false);
        isect2->setIsWfVert(sqrdTime, false);
        isect3->setIsWfVert(sqrdTime, false);
    } else if (bIsWfVert1 && bIsWfVert2 && !bIsWfVert3) {
        isect1->setIsWfVert(sqrdTime, false);
        isect2->setIsWfVert(sqrdTime, false);
        isect3->setIsWfVert(sqrdTime, true);
        m_nVorVerts++;
    }

    return true;
}

SitePtr VorDiag::otherSite(const SitePtr &site, const MovIsectPtr &isect) const {
    CGAL_assertion(isect->traj()->site1()->id() == site->id()
           || isect->traj()->site2()->id() == site->id());
    return isect->traj()->site1()->id() == site->id()
            ? isect->traj()->site2() : isect->traj()->site1();
}

MovIsectPtr VorDiag::makeMovIsectAt(const Circular_arc_point_2 &pnt,
                                    const SitePtr &site1, const SitePtr &site2) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("makeMovIsectAt");
    src::severity_logger<severity_level> slg;
#endif
    const auto &bisecId = std::minmax({site1->id(), site2->id()});
    
    if (m_bisecs.find(bisecId) != m_bisecs.end()) {
        const auto bisec = m_bisecs.at(bisecId);
        const auto traj = bisec->findTraj(pnt);
        const auto &trajId = std::make_tuple(traj->id().first, traj->id().second,
                                             traj->isLeft(), traj->isFirst());

        if (m_isects.find(trajId) == m_isects.end()) {
            m_isects[trajId] = std::make_shared<MovIsect>(traj);
        }

        return m_isects.at(trajId);
    }

#ifdef ENABLE_LOGGING
    BOOST_LOG_SEV(slg, error) << "Bisector (" << bisecId.first << ", "
            << bisecId.second << ") not found!";
#endif

    return MovIsectPtr{};
}

MovIsectPtr VorDiag::makeMovIsect(const TrajPtr &traj) {
    const auto &trajId = std::make_tuple(traj->id().first, traj->id().second,
                                         traj->isLeft(), traj->isFirst());

    if (m_isects.find(trajId) == m_isects.end()) {
        m_isects[trajId] = std::make_shared<MovIsect>(traj);
    }

    return m_isects.at(trajId);
}

void VorDiag::checkEdgeEv(const OffCircPtr &offCirc, const Root_of_2 &tNow,
                          const MovIsectPtr &isect, bool bLeft) {
    MovIsectPtr other;
    bool bOk = offCirc->neighbor(other, isect, bLeft);

    CGAL_assertion(bOk);
    if (bOk) {
        checkEdgeEv(offCirc, tNow, isect, other);
    }
}

void VorDiag::checkEdgeEv(const OffCircPtr &offCirc, const Root_of_2 &sqrdTimeNow,
                          const MovIsectPtr &isect, const MovIsectPtr &other) {
#ifdef ENABLE_LOGGING
    BOOST_LOG_NAMED_SCOPE("checkEdgeEv");
    src::severity_logger<severity_level> slg;
#endif

    auto temp = std::make_shared<TimePnt>(Point_2{},
                                          Root_of_2{std::numeric_limits<double>::max()});
    bool bIsValid = false;
    std::vector<TimePntPtr> verts;
    isect->traj()->isect(verts, other->traj());

    for (const auto &vert : verts) {
        if (vert->sqrdTime() < temp->sqrdTime()
            && sqrdTimeNow < vert->sqrdTime()) {
            temp = vert;
            bIsValid = true;
        }
    }
    
    if (bIsValid) {
        const auto &edgeEv = std::make_shared<EdgeEv>(temp->arcPnt(), temp->sqrdTime(), offCirc->site(), isect, other);
        m_queue.push(edgeEv);

#ifdef ENABLE_LOGGING    
        BOOST_LOG_SEV(slg, normal) << std::setprecision(30)
                << "Inserting edge event at time "
                << std::sqrt(CGAL::to_double(temp->sqrdTime())) << " at point ("
                << CGAL::to_double(temp->arcPnt().x()) << ", "
                << CGAL::to_double(temp->arcPnt().y()) << ") ... ";
#endif
    } else {
#ifdef ENABLE_LOGGING
        BOOST_LOG_SEV(slg, normal) << std::setprecision(30) 
                << "Invalid edge event at time "
                << std::sqrt(CGAL::to_double(temp->sqrdTime())) << " at point ("
                << CGAL::to_double(temp->arcPnt().x()) << ", "
                << CGAL::to_double(temp->arcPnt().y()) << ") has been discarded ... ";
#endif
    }
}

void VorDiag::storeEv(const EvPtr &ev) {
    switch(ev->type())
    {
        case EvType::Coll:
        {
            m_collEvCnt++;
            break;
        }
        case EvType::Dom:
        {
            m_domEvCnt++;
            break;
        }
        case EvType::Edge:
        {
            m_edgeEvCnt++;
            break;
        }
        case EvType::None:
        default:
        {
            break;
        }
    };
    
#ifdef ENABLE_VIEW
    m_evs.push_back(ev);
#endif
}

void VorDiag::compVorEdges() {
    for (const auto &val : m_isects) {
        const auto isect = val.second;
        const auto switches = isect->switches();
        bool bIsLeft = isect->traj()->isLeft();

        for (const auto trajSec : isect->traj()->secs()) {
            if (typeid(*trajSec) == typeid(TrajSecPntPnt)) {
                const auto tspp
                        = std::static_pointer_cast<TrajSecPntPnt>(trajSec);

                if (!tspp->isEqual()) {
                    const auto circ = tspp->arc().supporting_circle();

                    if (!switches.empty()) {
                        for (size_t i = 0; i < switches.size() - 1; i++) {
                            auto t1 = switches.at(i).first,
                                    t2 = switches.at(i + 1).first;
                            bool bIsWfEdge = switches.at(i).second;

                            if (bIsWfEdge && t1 != t2) {
                                const auto start = isect->pntAt(t1),
                                        end = isect->pntAt(t2);
                                const auto arc = Circular_arc_2{circ, bIsLeft ? start : end, bIsLeft ? end : start};
                                m_vorEdges.push_back(arc);
                            }
                        }
                    }
                } else {
                    if (!switches.empty()) {
                        for (size_t i = 0; i < switches.size() - 1; i++) {
                            auto t1 = switches.at(i).first,
                                    t2 = switches.at(i + 1).first;
                            bool bIsWfEdge = switches.at(i).second;

                            if (bIsWfEdge && t1 != t2) {
                                const auto start = isect->pntAt(t1),
                                        end = isect->pntAt(t2);
                                double x1 = CGAL::to_double(start.x()),
                                        y1 = CGAL::to_double(start.y()),
                                        x2 = CGAL::to_double(end.x()),
                                        y2 = CGAL::to_double(end.y());
                                m_vorSegs.push_back(Line_arc_2{Point_2{x1, y1}, Point_2{x2, y2}});
                            }
                        }
                    }
                }
            }
        }
    }
}

void VorDiag::writeToIpe(const std::string &filePath) const {
    auto iw = IpeWriter{filePath};
    double scale = 1e-4 / 6.;

    for (const auto &site : m_sites) {
        std::ostringstream strs;
        strs << "$s_" << site->id() 
                << "\\,(" << CGAL::to_double(site->weight() / 1e3) << ")$";

        if (typeid(*site) == typeid(PntSite)) {
            const auto pntSite = std::static_pointer_cast<PntSite>(site);
            iw.addIpeObj(IpeObj{pntSite->pnt(), "Sites", scale,
                         "seagreen", "large", strs.str()});
        }
    }

    for (const auto &vorEdge : m_vorEdges) {
        iw.addIpeObj(IpeObj{vorEdge, "VorDiag", scale, "black", "fat"});
    }
    
#ifdef ENABLE_VIEW
    std::vector<Root_of_2> evSqrdTimes;
    /*for (size_t i = 0; i < m_evs.size(); i++) {
        const auto ev = m_evs.at(i);
        std::stringstream strs;
        
        strs << "Event" << (i + 1);
        evSqrdTimes.push_back(ev->sqrdTime());
        iw.addIpeObj(IpeObj{ev->arcPnt(), strs.str(), scale, "red"});
    }*/
    
    const auto maxTime = FT{10},
            timeStep = FT{.2};
    for (const auto &val : m_offCircs) {
        const auto offCirc = val.second;
        offCirc->writeArcsToIpe(iw, maxTime, timeStep, scale, evSqrdTimes);
    }
#endif

    iw.write();
}

void VorDiag::writeToCsv(const std::string &filePath, long long runtime,
                         long long overlayDuration, long long eventDuration,
                         size_t maxCandSetSize, size_t nCandSets, size_t avgCandSetSize) const {
    std::ofstream outFile;
    outFile.open(filePath, std::ofstream::out | std::ofstream::app);

    if (outFile.is_open()) {
        outFile << m_sites.size() << "," << runtime << ","
                << overlayDuration << "," << eventDuration << ","
                << m_collEvCnt << "," << m_domEvCnt << "," << m_edgeEvCnt << ","
                << m_invalidCollEvCnt << "," << m_invalidDomEvCnt << ","
                << m_invalidEdgeEvCnt << "," << maxCandSetSize << ","
                << nCandSets << "," << avgCandSetSize << "," << m_nVorVerts
                << "\n";
    }
}

#ifdef ENABLE_VIEW

VorDiagGraphicsItem::VorDiagGraphicsItem(const VorDiag &vorDiag)
: m_vorDiag{vorDiag}
{
    prepareGeometryChange();
    QRectF rect;
    for (const auto &edge : m_vorDiag.edges()) {
        const auto circ = edge.supporting_circle();
        rect |= Util::boundingRect(circ);
    }
    
    m_boundingRect = rect;
}

QRectF VorDiagGraphicsItem::boundingRect() const {
    return m_boundingRect;
}

void VorDiagGraphicsItem::modelChanged() {
    update();
}

void VorDiagGraphicsItem::onToggle(bool bIsVisible) {
    setVisible(bIsVisible);
    update();
}


void VorDiagGraphicsItem::onPrevEv(double t) {
    const auto evs = m_vorDiag.events();
    if (m_evIndex > 0) {
        auto ev = evs.at(--m_evIndex);
        double evTime = CGAL::to_double(ev->sqrdTime());
        
        while (m_evIndex > 0 && evTime > t) {
            ev = evs.at(--m_evIndex);
            evTime = CGAL::to_double(ev->sqrdTime());
        }

        m_evPnt = ev->arcPnt();
        m_bEvPntVisible = true;
        emit timeChanged(std::sqrt(evTime));
    }
}

void VorDiagGraphicsItem::onNextEv(double t) {
    const auto evs = m_vorDiag.events();
    if (m_evIndex < evs.size() - 1) {
        auto ev = evs.at(++m_evIndex);
        double evTime = CGAL::to_double(ev->sqrdTime());

        while (m_evIndex < evs.size() - 1 && evTime < t) {
            ev = evs.at(++m_evIndex);
            evTime = CGAL::to_double(ev->sqrdTime());
        }

        m_evPnt = ev->arcPnt();
        m_bEvPntVisible = true;
        emit timeChanged(std::sqrt(evTime));
    }
}

void VorDiagGraphicsItem::onHideEvPnt() {
    m_bEvPntVisible = false;
}

void VorDiagGraphicsItem::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *, QWidget *) {
    const auto scale = std::max(painter->worldTransform().m11(), 
                                painter->worldTransform().m22());
    auto brush = QBrush{Qt::black};
    painter->setBrush(brush);
    painter->setPen(QPen{brush, 1. / scale});

    for (const auto &edge : m_vorDiag.edges()) {
        Util::draw(painter, edge);
    }
    
    for (const auto &edge : m_vorDiag.segs()) {
        Util::draw(painter, edge);
    }

    if (m_bEvPntVisible) {
        brush = QBrush{Qt::red};
        painter->setBrush(brush);
        Util::draw(painter, m_evPnt, 8.);
    }
}

#include "moc_mwvd.cpp"

#endif
