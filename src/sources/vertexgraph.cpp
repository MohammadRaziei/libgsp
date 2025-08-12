//
// Created by mohammad on 8/12/25.
//

#include "libgsp/graph/vertexgraph.h"



gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes) {}

void gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    this->names = names;
}
void gsp::VertexGraph::setCoords(const Eigen::MatrixXd& coords) {
    this->coords = coords;
}
void gsp::VertexGraph::setCoords(const std::vector<gsp::Coord>& src) {
    assert(src.size() == num_nodes && "coords size mismatch");

    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>>
        mapped(reinterpret_cast<const double*>(src.data()), num_nodes, 3);

    coords = mapped; // one contiguous copy
}
gsp::VertexGraph::~VertexGraph() {}

