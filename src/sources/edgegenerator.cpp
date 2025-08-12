//
// Created by mohammad on 8/12/25.
//

#include "libgsp/graph/edgegenerator.h"
#include "libgsp/graph/graph.h"


#include <cmath>
#include <Eigen/Dense>
#include <Eigen/SparseCore>

namespace gsp {

// ===================== densematrix =====================
template <class Matrix>
EdgeGenerator<Matrix>::EdgeGenerator(const gsp::Graph<Matrix>* graph, types::elem_t<Matrix> thresh) {
    _weights  = graph ? &graph->weights() : nullptr;
    _num_nodes= graph ? static_cast<int>(graph->num_nodes) : 0;
    _thresh      = thresh;
    _is_directed = graph ? graph->isDirected() : false;
    _state = new State;
    iter();
}


template <class Matrix>
EdgeGenerator<Matrix>::~EdgeGenerator() {
    delete _state;
}

template <>
void EdgeGenerator<densematrix>::iter() {
    _state->reset();
}

template <>
void EdgeGenerator<sparsematrix>::iter() {
    // outer_ = 0;
    // k_ = (W_ && W_->outerSize() > 0) ? outerPtr_[0] : 0;
}

template <>
std::optional<Edge> EdgeGenerator<densematrix>::next() {
    if (!_weights || _num_nodes <= 0) return std::nullopt;

    while (_state->row < _num_nodes) {
        // for undirected we emit only upper triangle: col starts at row
        while (_state->col < _num_nodes) {
            const uint32_t col = _state->col++;               // advance state BEFORE possible return
            const double w = (*_weights)(_state->row, col);

            if (std::abs(w) <= _thresh) continue;

            return Edge(_state->row, col, w);
        }
        ++_state->row;
        _state->col = (_is_directed) ? 0 : _state->row; // reset for next row
    }
    return std::nullopt;
}




template <>
std::optional<Edge> EdgeGenerator<sparsematrix>::next() {
    if (!_weights || _num_nodes <= 0) return std::nullopt;


    return std::nullopt;
}


template <class Matrix>
std::vector<Edge> EdgeGenerator<Matrix>::toVector() {
    std::vector<Edge> edges;
    const size_t max_num = (_is_directed) ? _num_nodes * _num_nodes : _num_nodes * (_num_nodes + 1) / 2; // max edges in undirected graph
    edges.reserve(max_num);
    iter();
    while (auto edge = next()) {
        edges.push_back(*edge);
    }
    return edges;
}

// Optional explicit instantiation to keep symbols in TU (if needed):
template class EdgeGenerator<densematrix>;
template class EdgeGenerator<sparsematrix>;

} // namespace gsp
