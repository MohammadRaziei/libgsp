//
// Created by mohammad on 8/12/25.
//

#include "libgsp/graph/edgegenerator.h"
#include "libgsp/graph/graph.h"


#include <cmath>
#include <Eigen/Dense>
#include <Eigen/SparseCore>

namespace gsp {

template <class Matrix>
EdgeGenerator<Matrix>::EdgeGenerator(const gsp::Graph<Matrix>* graph, types::elem_t<Matrix> thresh) {
    _weights  = graph ? &graph->weights() : nullptr;
    _num_nodes= graph ? static_cast<int>(graph->num_nodes) : 0;
    _thresh      = thresh;
    _is_directed = graph ? graph->isDirected() : false;
    _state = new State(_weights);
    iter();
}


template <class Matrix>
EdgeGenerator<Matrix>::~EdgeGenerator() {
    delete _state;
}

template <class Matrix>
void EdgeGenerator<Matrix>::iter() {
    _state->reset();
}


template <>
std::optional<Edge> EdgeGenerator<densematrix>::next() {
    if (!_weights || _num_nodes <= 0) return std::nullopt;

    while (_state->_row < _num_nodes) {
        // for undirected we emit only upper triangle: col starts at row
        while (_state->_col < _num_nodes) {
            const uint32_t col = _state->_col++;               // advance state BEFORE possible return
            const double w = (*_weights)(_state->_row, col);

            if (std::abs(w) <= _thresh) continue;

            return Edge(_state->_row, col, w);
        }
        ++_state->_row;
        _state->_col = (_is_directed) ? 0 : _state->_row; // reset for next row
    }
    return std::nullopt;
}





template <>
std::optional<Edge> EdgeGenerator<sparsematrix>::next() {
    if (!_weights || _num_nodes <= 0) return std::nullopt;

    // Iterate row-by-row over CSR buffers
    while (_state->_row < _num_nodes) {
        while (_state->_k < _state->_kend) {
            const uint32_t idx = _state->_k++;
            const uint32_t r   = _state->_row;
            const uint32_t c   = _state->_colIdx[idx];
            const double w     = _state->_values[idx];

            // undirected: only i <= j
            if (!_is_directed && r > c) continue;

            if (std::abs(w) <= _thresh) continue;

            return Edge(r, c, w);
        }
        // next row
        ++_state->_row;
        if (_state->_row < _num_nodes && _state->_rowPtr) {
            _state->reset_row();
        }
    }
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
