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
    if (!_weights || _num_nodes <= 0 || !_state) return std::nullopt;

    auto* st = _state; // convenience

    while (st->W && st->outer < st->W->outerSize()) {
        if (!st->it) st->advance_to_next_nonempty_row();
        if (!st->it || !(*st->it)) {
            // this row exhausted -> move to next row
            ++st->outer;
            st->it.reset();
            st->advance_to_next_nonempty_row();
            continue;
        }

        // snapshot current entry, advance iterator state BEFORE checks
        const int    r = st->it->row();
        const int    c = st->it->col();
        const double w = st->it->value();
        ++(*st->it);

        // undirected: keep only upper triangle (i <= j)
        if (!_is_directed && r > c) continue;

        if (std::abs(w) <= static_cast<double>(_thresh)) continue;

        return Edge(static_cast<uint32_t>(r),
                    static_cast<uint32_t>(c),
                    w);
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
