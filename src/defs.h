#ifndef DEFS_H
#define DEFS_H

#include <CGAL/Exact_circular_kernel_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Simple_cartesian.h>

using CK = CGAL::Exact_circular_kernel_2;
using Point_2 = CGAL::Point_2<CK>;
using Circle_2 = CGAL::Circle_2<CK>;
using Circular_arc_2 = CGAL::Circular_arc_2<CK>;
using Circular_arc_point_2 = CGAL::Circular_arc_point_2<CK>;
using Line_2 = CGAL::Line_2<CK>;
using Line_arc_2 = CGAL::Line_arc_2<CK>;
using Ray_2 = CGAL::Ray_2<CK>;
using Segment_2 = CGAL::Segment_2<CK>;
using FT = CK::FT;
using Root_of_2 = CK::Root_of_2;

#endif /* DEFS_H */
