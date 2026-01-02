//
// Created by mohammad on 8/12/25.
//

#include "libgsp/VertexGraph.h"
#include "libgsp/CommonTypes.h"
#include <cassert>

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

std::vector<gsp::Node> gsp::VertexGraph::nodes() const {
    std::vector<gsp::Node> vec;
    vec.reserve(num_nodes);
    for (uint32_t i = 0; i < num_nodes; ++i) {
        vec.emplace_back(i, getCoord(i), getName(i));
    }
    return vec;
 }


 std::string gsp::VertexGraph::getName(uint32_t idx) const{
     return (!names.empty()) ? names[idx] : "v"+std::to_string(idx);
 }

 gsp::Coord gsp::VertexGraph::getCoord(uint32_t idx) const{
     gsp::Coord coord = (_coords.rows() > 0)
                            ? *reinterpret_cast<const gsp::Coord*>(_coords.row(idx).data())
                            : gsp::Coord(NAN, NAN, NAN);
     return coord;
 }


