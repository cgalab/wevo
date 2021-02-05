#ifndef OVERLAY_H
#define OVERLAY_H

#include <CGAL/Cartesian.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Arr_default_overlay_traits.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Voronoi_diagram_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_traits_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_policies_2.h>

class Overlay {
    struct OverlayLabel {
        std::set<int> operator()(const std::set<int> &lhs,
                const std::set<int> &rhs) const;
    };

    using Kernel = CGAL::Cartesian<CGAL::Exact_rational>;
    using FT = Kernel::FT;
    using Traits_2 = CGAL::Arr_segment_traits_2<Kernel>;
    using Dcel = CGAL::Arr_face_extended_dcel<Traits_2, std::set<int>>;
    using Arrangement_2 = CGAL::Arrangement_2<Traits_2, Dcel>;
    using Overlay_traits = CGAL::Arr_face_overlay_traits<Arrangement_2,
            Arrangement_2, Arrangement_2, OverlayLabel>;
    using DT = CGAL::Delaunay_triangulation_2<Kernel>;
    using AT = CGAL::Delaunay_triangulation_adaptation_traits_2<DT>;
    using AP = CGAL::Delaunay_triangulation_caching_degeneracy_removal_policy_2<DT>;
    using VD = CGAL::Voronoi_diagram_2<DT, AT, AP>;
    using Point_2 = Kernel::Point_2;
    using Segment_2 = Traits_2::X_monotone_curve_2;
    using Site_2 = AT::Site_2;
    
public:
    Overlay(const std::string &filePath);

    const std::vector<std::set<int>> & candSets() const {
        return m_candSets;
    }

    // Returns the overall runtime (without file I/O) in microseconds.
    long long duration() const {
        return m_duration;
    }

    size_t maxSize() const {
        return m_maxSize;
    }
    
    size_t avgSize() const {
        return m_avgSize;
    }
    
private:
    void readInput(const std::string &filePath);
    void compOverlay();

    std::vector<Site_2> m_sites;
    std::vector<std::set<int>> m_candSets;
    long long m_duration;
    size_t m_maxSize{0};
    size_t m_avgSize{0};
};

#endif /* OVERLAY_H */
