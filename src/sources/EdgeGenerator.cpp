//
// Created by Mohammad on 1/3/2026.
//

#include "libgsp/iterators/EdgeGenerator.h"
#include "libgsp/BaseGraph.h"

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <cmath>

namespace gsp {

// Dense matrix state implementation
template <>
struct EdgeGenerator<densematrix>::State {
    State(const densematrix*) { reset(); }
    void reset() { _row = _col = 0; }
    uint32_t _row, _col;
};

// Sparse matrix state implementation
template <>
struct EdgeGenerator<sparsematrix>::State {
    using InnerIt = sparsematrix::InnerIterator;

    explicit State(const sparsematrix* W) : W(W) { reset(); }

    void reset() {
        outer = 0;
        it.reset();
        // jump to first non-empty row
        advance_to_next_nonempty_row();
    }

    void advance_to_next_nonempty_row() {
        if (!W) return;
        const int outerSize = W->outerSize(); // == rows for RowMajor
        while (outer < outerSize) {
            it = std::make_unique<InnerIt>(*W, outer);
            if (*it) break;   // row has at least one nnz
            ++outer;          // try next row
        }
    }

    const sparsematrix* W = nullptr;
    uint32_t outer = 0;                          // current row
    std::unique_ptr<InnerIt> it;            // iterator within current row
};

// Constructor
template <class Matrix>
EdgeGenerator<Matrix>::EdgeGenerator(const Matrix* weights, int num_nodes, bool is_directed) 
    : _weights(weights), _num_nodes(num_nodes), _is_directed(is_directed) {
    _state = std::make_unique<State>(weights);
    reset(); // Initialize with default threshold
}

// Destructor
template <class Matrix>
EdgeGenerator<Matrix>::~EdgeGenerator() = default;

// Reset with threshold
template <class Matrix>
void EdgeGenerator<Matrix>::reset(types::elem_t<Matrix> thresh) {
    _thresh = thresh;
    _state->reset();
}

// Next edge for dense matrix
template <>
std::optional<Edge> EdgeGenerator<densematrix>::next() {
    if (!_weights || _num_nodes <= 0 || !_state || _weights->rows() == 0) return std::nullopt;

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

// Next edge for sparse matrix
template <>
std::optional<Edge> EdgeGenerator<sparsematrix>::next() {
    if (!_weights || _num_nodes <= 0 || !_state || _weights->rows() == 0) return std::nullopt;

    auto* st = _state.get(); // convenience

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
        const Eigen::Index r = st->it->row();
        const Eigen::Index c = st->it->col();
        const double       w = st->it->value();
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

// Explicit instantiations
template class EdgeGenerator<densematrix>;
template class EdgeGenerator<sparsematrix>;

} // namespace gsp