#include <iostream>
#include <chrono>
#include <deque>
#include <fstream>
#include <regex>
#include <math.h>
#ifdef ENABLE_VIEW
#include <QString>
#endif
#include "overlay.h"

std::set<int> Overlay::OverlayLabel::operator()(const std::set<int> &lhs,
                                                const std::set<int> &rhs) const {
    std::set<int> res;
    res.insert(lhs.begin(), lhs.end());
    res.insert(rhs.begin(), rhs.end());
    return res;
}

Overlay::Overlay(const std::string &filePath) {
    std::cout << "Computing overlay arrangement ...\n";
    readInput(filePath);
    
    std::chrono::high_resolution_clock::time_point t0 =
            std::chrono::high_resolution_clock::now();
    compOverlay();
    std::chrono::high_resolution_clock::time_point t1 =
            std::chrono::high_resolution_clock::now();
    m_duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
}

void Overlay::readInput(const std::string &filePath) {
    std::ifstream file;
    file.open(filePath, std::ifstream::in);

    const auto fSplit
            = [](const std::string &str, const std::regex &re) {
                return std::vector<std::string>{std::sregex_token_iterator(str.begin(), str.end(), re, -1),
                                                std::sregex_token_iterator()};
            };

    const auto fComp
            = [](const std::tuple<int, int, int> &lhs,
                 const std::tuple<int, int, int> &rhs) {
                return std::get<2>(lhs) > std::get<2>(rhs);
            };
    std::multiset<std::tuple<int, int, int>, decltype(fComp)> temps(fComp);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            const auto strs = fSplit(line, std::regex("\\s+"));
            CGAL_assertion(strs.size() == 3);
#ifdef ENABLE_VIEW
            int x = QString::fromStdString(strs.at(0)).toInt(),
                    y = QString::fromStdString(strs.at(1)).toInt(),
                    w = QString::fromStdString(strs.at(2)).toInt();
#else
            int x = std::stoi(strs.at(0)),
                    y = std::stoi(strs.at(1)),
                    w = std::stoi(strs.at(2));
#endif

            temps.insert(std::make_tuple(x, y, w));
        }
    }

    for (const auto temp : temps) {
        int x = std::get<0>(temp), y = std::get<1>(temp);
        m_sites.push_back(Point_2{x, y});
    }
}

void Overlay::compOverlay() {
    if (m_sites.empty()) {
        return;
    }
    
    Arrangement_2 overlayArr;
    std::deque<Arrangement_2> arrs;
    VD vd;
    int i = 0;

    const auto fMergeArrs
            = [](const Arrangement_2 &arr1, const Arrangement_2 & arr2) {
                Arrangement_2 res;
                Overlay_traits overlayTraits;
                CGAL::overlay(arr1, arr2, res, overlayTraits);
                return res;
            };
            
    const auto fCompOverlay
            = [&](std::deque<Arrangement_2> arrs) {
                while (arrs.size() > 1) {
                    std::cout << "\tMerging " << arrs.size() << " arrangements ...\n";
                    std::deque<Arrangement_2> temps;
                    for (int i = arrs.size() - 1; i >= 0; i -= 2) {
                        const auto arr1 = arrs.at(i);
                        Arrangement_2 arr2 = (i == 0) 
                                ? temps.at(0) : arrs.at(i - 1);

                        const auto res = fMergeArrs(arr1, arr2);
                        temps.push_back(res);

                        if (i == 0) {
                            temps.pop_front();
                        }
                    }
                    
                    std::swap(arrs, temps);
                    temps.clear();
                }

                CGAL_assertion(arrs.size() == 1);
                overlayArr = arrs.front();
            };

    vd.insert(Point_2{std::numeric_limits<int>::max(),
              std::numeric_limits<int>::max()});
    vd.insert(Point_2{std::numeric_limits<int>::max(),
              std::numeric_limits<int>::min()});
    vd.insert(Point_2{std::numeric_limits<int>::min(),
              std::numeric_limits<int>::max()});
    vd.insert(Point_2{std::numeric_limits<int>::min(),
              std::numeric_limits<int>::min()});
              
    for (const auto &site : m_sites) {
        const auto face = vd.insert(site);
        Arrangement_2 arr;
        const auto hcStart = face->ccb();
        auto hc = hcStart;
        
        do {
            CGAL_assertion(hc->has_source() && hc->has_target());
            const auto pnt1 = hc->source()->point(),
                    pnt2 = hc->target()->point();
            insert_non_intersecting_curve(arr, Segment_2{pnt1, pnt2});
        } while (++hc != hcStart);

        Arrangement_2::Face_iterator fit;
        for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
            bool bIsUnbounded = fit == arr.unbounded_face();
            fit->set_data(std::set<int>{bIsUnbounded ? -1 : i});
        }

        arrs.push_back(arr);
        i++;
    }

    fCompOverlay(arrs);

    Arrangement_2::Face_iterator fit;
    double cumulatedSize = 0.;
    for (fit = overlayArr.faces_begin();
         fit != overlayArr.faces_end(); ++fit) {
        const auto candSet = fit->data();
        m_candSets.push_back(candSet);

        cumulatedSize += candSet.size();
        if (candSet.size() > m_maxSize) {
            m_maxSize = candSet.size();
        }
    }

    m_avgSize = std::round(cumulatedSize / m_candSets.size());
    std::cout << "The largest candidate size has size " << m_maxSize << ".\n"
            << "The average candidate size has size " << m_avgSize << ".\n"
            << "There are " << m_candSets.size() << " candidate sets.\n";
}
