#ifndef GRAPHML_H
#define GRAPHML_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>

struct VertexProperty {
    std::string x;
    std::string y;
    std::string weight;
    using kind = boost::vertex_property_tag;
};

struct EdgeProperty {
    using kind = boost::edge_property_tag;
};

class GraphmlData {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperty, EdgeProperty>;
    
public:
    using vertex_property_type = Graph::vertex_property_type;
    using vertex_descriptor = Graph::vertex_descriptor;
    
    GraphmlData(std::istream &istream);
    const Graph & graph() const {
        return m_graph;
    }
    
private:
    Graph m_graph;
    boost::dynamic_properties m_dp;
};

#endif /* GRAPHML_H */

