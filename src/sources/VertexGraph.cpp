//
// Created by mohammad on 8/12/25.
//

#include "libgsp/VertexGraph.h"
#include "libgsp/CommonTypes.h"
#include <cassert>

gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes), _coords(Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>::Zero(num_nodes, 3)), _state_vertex(0) {}

void gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    assert(names.size() == num_nodes && "names size mismatch");
    this->names = names;
}
void gsp::VertexGraph::setCoords(const Eigen::MatrixXd& coords) {
    assert(coords.rows() == num_nodes && coords.cols() == 3 && "coords size mismatch");
    this->_coords = coords;
}
void gsp::VertexGraph::setCoords(const std::vector<gsp::Coord>& src) {
    assert(src.size() == num_nodes && "coords size mismatch");

    _coords.resize(num_nodes, 3);
    for (uint32_t i = 0; i < num_nodes; ++i) {
        _coords(i, 0) = src[i].x;
        _coords(i, 1) = src[i].y;
        _coords(i, 2) = src[i].z;
    }
}

gsp::VertexGraph::CoordMat gsp::VertexGraph::coords() {
    return _coords;
}
gsp::VertexGraph::~VertexGraph() {
}

std::vector<gsp::Node> gsp::VertexGraph::nodes() const {
    std::vector<gsp::Node> vec;
    vec.reserve(num_nodes);
    for (uint32_t i = 0; i < num_nodes; ++i) {
        vec.emplace_back(i, getCoord(i), getName(i));
    }
    return vec;
 }


 std::string gsp::VertexGraph::getName(uint32_t idx) const{
     return (!names.empty() && idx < names.size()) ? names[idx] : "v"+std::to_string(idx);
 }

 gsp::Coord gsp::VertexGraph::getCoord(uint32_t idx) const{
     if (_coords.rows() > 0 && idx < static_cast<uint32_t>(_coords.rows())) {
         return gsp::Coord(_coords(idx, 0), _coords(idx, 1), _coords(idx, 2));
     } else {
         return gsp::Coord(NAN, NAN, NAN);
     }
 }


