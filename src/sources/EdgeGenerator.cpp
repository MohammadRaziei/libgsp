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
    void reset() { row_ = col_ = 0; }
    uint32_t row_, col_;
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

// Constructor with threshold
template <class Matrix>
EdgeGenerator<Matrix>::EdgeGenerator(const Matrix* weights, int num_nodes, bool is_directed, types::elem_t<Matrix> thresh)
    : weights_(weights), num_nodes_(num_nodes), is_directed_(is_directed) {
        state_ = std::make_unique<State>(weights);
        thresh_ = thresh;  // Set the threshold
    state_->reset();   // Reset the state
}

// Destructor
template <class Matrix>
EdgeGenerator<Matrix>::~EdgeGenerator() = default;

// Reset (keeps current threshold)
template <class Matrix>
void EdgeGenerator<Matrix>::reset() {
    state_->reset();
}

// Next edge for dense matrix
template <>
std::optional<Edge> EdgeGenerator<densematrix>::next() {
    if (!weights_ || num_nodes_ <= 0 || !state_ || weights_->rows() == 0) return std::nullopt;

    while (state_->row_ < num_nodes_) {
        // for undirected we emit only upper triangle: col starts at row
        while (state_->col_ < num_nodes_) {
            const uint32_t col = state_->col_++;               // advance state BEFORE possible return
            const double w = (*weights_)(state_->row_, col);

            if (std::abs(w) <= thresh_) continue;

            return Edge(state_->row_, col, w);
        }
        ++state_->row_;
        state_->col_ = (is_directed_) ? 0 : state_->row_; // reset for next row
    }
    return std::nullopt;
}

// Next edge for sparse matrix
template <>
std::optional<Edge> EdgeGenerator<sparsematrix>::next() {
    if (!weights_ || num_nodes_ <= 0 || !state_ || weights_->rows() == 0) return std::nullopt;

    auto* st = state_.get(); // convenience

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
        if (!is_directed_ && r > c) continue;

        if (std::abs(w) <= static_cast<double>(thresh_)) continue;

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