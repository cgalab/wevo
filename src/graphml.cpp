#include "graphml.h"

GraphmlData::GraphmlData(std::istream &istream)
: m_dp{boost::ignore_other_properties}
{
  m_dp.property("vertex-coordinate-x", boost::get(&VertexProperty::x, m_graph));
  m_dp.property("vertex-coordinate-y", boost::get(&VertexProperty::y, m_graph));
  m_dp.property("vertex-weight", boost::get(&VertexProperty::weight, m_graph));

  boost::read_graphml(istream, m_graph, m_dp);
}
