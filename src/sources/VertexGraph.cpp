//
// Created by mohammad on 8/12/25.
//

#include "libgsp/VertexGraph.h"
#include <cassert>

gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes), _coords(Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>::Zero(num_nodes, 3)) {}

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
        _coords(i, 0) = src[i].x();
        _coords(i, 1) = src[i].y();
        _coords(i, 2) = src[i].z();
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
        vec.emplace_back(i, coord(i), name(i));
    }
    return vec;
 }


 const std::string gsp::VertexGraph::name(uint32_t idx) const{
     return (!names.empty() && idx < names.size()) ? names[idx] : "v"+std::to_string(idx);
 }

const gsp::Coord gsp::VertexGraph::coord(uint32_t idx) const{
    if (_coords.rows() > 0 && idx < static_cast<uint32_t>(_coords.rows())) {
        return gsp::Coord(_coords(idx, 0), _coords(idx, 1), _coords(idx, 2), const_cast<VertexGraph*>(this), idx);
    } else {
        return gsp::Coord(NAN, NAN, NAN, const_cast<VertexGraph*>(this), idx);
    }
}

gsp::Coord gsp::VertexGraph::coord(uint32_t idx) {
     if (_coords.rows() > 0 && idx < static_cast<uint32_t>(_coords.rows())) {
         return gsp::Coord(_coords(idx, 0), _coords(idx, 1), _coords(idx, 2), const_cast<VertexGraph*>(this), idx);
     } else {
         return gsp::Coord(NAN, NAN, NAN, const_cast<VertexGraph*>(this), idx);
     }
 }


void gsp::VertexGraph::setCoord(uint32_t idx, const Coord &coord) {
    if (idx >= num_nodes) {
        throw std::out_of_range("out of the range!");
    }
    if (_coords.rows() == 0) {
        _coords.resize(num_nodes, 3);
    }
    _coords(idx, 0) = coord.x();
    _coords(idx, 1) = coord.y();
    _coords(idx, 2) = coord.z();
}

void gsp::VertexGraph::setCoord(uint32_t row, uint32_t col, double value) {
    if (row >= num_nodes || col >= 3) {
        throw std::out_of_range("out of the range!");
    }
    if (_coords.rows() == 0) {
        _coords.resize(num_nodes, 3);
    }
    _coords(row, col) = value;
}


gsp::VertexIterator gsp::VertexGraph::begin() {
    return VertexIterator(this, 0);
}

gsp::VertexIterator gsp::VertexGraph::end() {
    return VertexIterator(this, num_nodes);
}

gsp::ConstVertexIterator gsp::VertexGraph::begin() const {
    return ConstVertexIterator(this, 0);
}

gsp::ConstVertexIterator gsp::VertexGraph::end() const {
    return ConstVertexIterator(this, num_nodes);
}

gsp::ConstVertexIterator gsp::VertexGraph::cbegin() const {
    return ConstVertexIterator(this, 0);
}

gsp::ConstVertexIterator gsp::VertexGraph::cend() const {
    return ConstVertexIterator(this, num_nodes);
}

std::reverse_iterator<gsp::VertexIterator> gsp::VertexGraph::rbegin() {
    return std::reverse_iterator<VertexIterator>(end());
}

std::reverse_iterator<gsp::VertexIterator> gsp::VertexGraph::rend() {
    return std::reverse_iterator<VertexIterator>(begin());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::rbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(end());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::rend() const {
    return std::reverse_iterator<ConstVertexIterator>(begin());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::crbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(cend());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::crend() const {
    return std::reverse_iterator<ConstVertexIterator>(cbegin());
}


void gsp::Coord::setX(double x) {
    x_ = x;
    if (graph_) {
        graph_->setCoord(idx_, 0, x);
    }
}

void gsp::Coord::setY(double y) {
    y_ = y;
    if (graph_) {
        graph_->setCoord(idx_, 1, y);
    }
}

void gsp::Coord::setZ(double z) {
    z_ = z;
    if (graph_) {
        graph_->setCoord(idx_, 2, z);
    }
}