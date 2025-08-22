//
// Created by mohammad on 8/12/25.
//

#include "libgsp/graph/vertexgraph.h"



gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes), _state_vertex(0) {}

void gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    assert(names.size() == num_nodes && "names size mismatch");
    this->names = names;
}
void gsp::VertexGraph::setCoords(const Eigen::MatrixXd& coords) {
    assert(coords.size() == num_nodes && "coords size mismatch");
    this->_coords = coords;
}
void gsp::VertexGraph::setCoords(const std::vector<gsp::Coord>& src) {
    assert(src.size() == num_nodes && "coords size mismatch");

    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>>
        mapped(reinterpret_cast<const double*>(src.data()), num_nodes, 3);

    _coords = mapped; // one contiguous copy
}

gsp::VertexGraph::CoordMat gsp::VertexGraph::coords() {
    return _coords;
}
gsp::VertexGraph::~VertexGraph() {
}
void gsp::VertexGraph::nodeIter() {
    _state_vertex = 0;
}
std::optional<gsp::Node> gsp::VertexGraph::nodeNext() {
    if (_state_vertex >= num_nodes) {
        return std::nullopt;
    }


    gsp::Node node(_state_vertex,
                   getCoord(_state_vertex),
                   getName(_state_vertex));
    ++_state_vertex;
    return std::optional<gsp::Node>({std::move(node)});
}

std::vector<gsp::Node> gsp::VertexGraph::nodes() {
    nodeIter();
    std::vector<gsp::Node> vec;
    vec.reserve(num_nodes);
    while (auto node = nodeNext()) {
        vec.push_back(*node);
    }
    return vec;
 }


 std::string gsp::VertexGraph::getName(uint32_t idx) {
     return (!names.empty()) ? names[idx] : "v"+std::to_string(idx);
 }

 gsp::Coord gsp::VertexGraph::getCoord(uint32_t idx) {
     gsp::Coord coord = (_coords.rows() > 0)
                            ? *reinterpret_cast<gsp::Coord*>(_coords.row(idx).data())
                            : gsp::Coord(NAN, NAN, NAN);
     return coord;
 }


