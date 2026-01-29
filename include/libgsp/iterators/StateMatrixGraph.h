//
// gsp::StateMatrixGraph — generic Eigen dense/sparse edge generator
// - Works with Eigen::Matrix<Scalar,...> and Eigen::SparseMatrix<Scalar,...>
// - Prototypes first, implementations below
// - Member variables are snake_case
// - thresh_ stays double
// - Uses gsp::types::{elem_t,is_eigen_dense,is_eigen_sparse,float_of}
// - Edge now stores std::shared_ptr<gsp::BaseMiniState> (instead of cloned generator)
// - BaseMiniState exposes: double value() const, void setValue(double)
//

#ifndef LIBGSP_STATEMATRIXGRAPH_H
#define LIBGSP_STATEMATRIXGRAPH_H
#pragma once

#include "libgsp/iterators/EdgeGenerator.h"
#include "libgsp/utils/Types.h"

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

namespace gsp {

template <class Matrix>
class StateMatrixGraph final : public gsp::BaseStateEdgeGenerator {
public:
    using matrix_type = Matrix;
    using scalar_type = gsp::types::elem_t<Matrix>;
    using float_type  = gsp::types::float_of<scalar_type>;

    StateMatrixGraph(Matrix* weights, bool is_directed, double thresh);
    StateMatrixGraph(const StateMatrixGraph&) = delete;
    StateMatrixGraph& operator=(const StateMatrixGraph&) = delete;
    ~StateMatrixGraph() override = default;

    void reset() override;
    std::optional<gsp::Edge> next() override;


private:
    // ---- matrix category ----
    static constexpr bool is_dense_ =
        gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value;

    static constexpr bool is_sparse_ =
        gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value;

    static_assert(is_dense_ || is_sparse_,
                  "StateMatrixGraph<Matrix>: Matrix must be an Eigen dense Matrix or Eigen SparseMatrix.");

    // ---- state (conditional) ----
    struct dense_state final : gsp::BaseMiniState {
        explicit dense_state(Matrix* weights);

        void reset(bool is_directed, uint32_t num_nodes);

        double value() const override;
        void setValue(double v) override;

        uint32_t row = 0;
        uint32_t col = 0;

        // last emitted position (Edge should operate on this)
        uint32_t last_row = 0;
        uint32_t last_col = 0;

        Matrix* weights = nullptr;
        bool is_directed = false;
        uint32_t num_nodes = 0;
    };

    struct sparse_state final : gsp::BaseMiniState {
        using sparse_matrix  = std::remove_cv_t<std::remove_reference_t<Matrix>>;
        using inner_iterator = typename sparse_matrix::InnerIterator;

        explicit sparse_state(Matrix* weights);

        void reset();
        void advance_to_next_nonempty_outer();

        double value() const override;
        void setValue(double v) override;

        Matrix* weights = nullptr;

        uint32_t outer = 0;
        std::optional<inner_iterator> it;      // live iterator
        std::optional<inner_iterator> last_it; // last emitted entry (Edge operates on this)
    };

    using state_t = std::conditional_t<is_dense_, dense_state, sparse_state>;

    // ---- data members (snake_case) ----
    Matrix*  weights_     = nullptr;
    uint32_t num_nodes_   = 0;
    double   thresh_      = 0.0;
    bool     is_directed_ = false;

    // current traversal state
    std::shared_ptr<state_t> state_;

private:
    // ---- internal helpers ----
    void reset_dense_();
    void reset_sparse_();

    std::optional<gsp::Edge> next_dense_();
    std::optional<gsp::Edge> next_sparse_();

    // make a snapshot of the current state object for Edge ownership
    std::shared_ptr<gsp::BaseMiniState> snapshot_state_for_edge_() const;
};

} // namespace gsp

// ============================
// Implementations
// ============================

namespace gsp {

// ---------- dense_state ----------

template <class Matrix>
StateMatrixGraph<Matrix>::dense_state::dense_state(Matrix* w)
    : weights(w) {}

template <class Matrix>
void StateMatrixGraph<Matrix>::dense_state::reset(bool directed, uint32_t n) {
    row = 0;
    col = 0;
    last_row = 0;
    last_col = 0;

    is_directed = directed;
    num_nodes = n;
}

template <class Matrix>
double StateMatrixGraph<Matrix>::dense_state::value() const {
    return static_cast<double>(
        (*weights)(static_cast<Eigen::Index>(last_row),
                   static_cast<Eigen::Index>(last_col))
    );
}

template <class Matrix>
void StateMatrixGraph<Matrix>::dense_state::setValue(double v) {
    (*weights)(static_cast<Eigen::Index>(last_row),
               static_cast<Eigen::Index>(last_col)) =
        static_cast<scalar_type>(static_cast<float_type>(v));
}

// ---------- sparse_state ----------

template <class Matrix>
StateMatrixGraph<Matrix>::sparse_state::sparse_state(Matrix* w)
    : weights(w) {}

template <class Matrix>
void StateMatrixGraph<Matrix>::sparse_state::reset() {
    outer = 0;
    it.reset();
    last_it.reset();
    advance_to_next_nonempty_outer();
}

template <class Matrix>
void StateMatrixGraph<Matrix>::sparse_state::advance_to_next_nonempty_outer() {
    if (!weights) return;

    const int outer_size = weights->outerSize(); // RowMajor: rows
    while (static_cast<int>(outer) < outer_size) {
        it.emplace(*weights, static_cast<int>(outer));
        if (*it) return;
        it.reset();
        ++outer;
    }
}

template <class Matrix>
double StateMatrixGraph<Matrix>::sparse_state::value() const {
    // Caller should only use after at least one edge was emitted.
    return static_cast<double>(last_it->value());
}

template <class Matrix>
void StateMatrixGraph<Matrix>::sparse_state::setValue(double v) {
    // Caller should only use after at least one edge was emitted.
    last_it->valueRef() = static_cast<scalar_type>(static_cast<float_type>(v));
}

// ---------- StateMatrixGraph ----------

template <class Matrix>
StateMatrixGraph<Matrix>::StateMatrixGraph(Matrix* weights, bool is_directed, double thresh)
    : weights_(weights),
      num_nodes_(weights ? static_cast<uint32_t>(weights->rows()) : 0),
      thresh_(thresh),
      is_directed_(is_directed),
      state_(std::make_shared<state_t>(weights)) {
    reset();
}

template <class Matrix>
void StateMatrixGraph<Matrix>::reset() {
    if (!state_) return;

    if constexpr (is_dense_) {
        reset_dense_();
    } else {
        reset_sparse_();
    }
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next() {
    if (!weights_ || num_nodes_ == 0 || weights_->rows() == 0 || !state_) return std::nullopt;

    if constexpr (is_dense_) {
        return next_dense_();
    } else {
        return next_sparse_();
    }
}


template <class Matrix>
std::shared_ptr<gsp::BaseMiniState> StateMatrixGraph<Matrix>::snapshot_state_for_edge_() const {
    // Create an owning snapshot that "pins" the last emitted position for Edge::setWeight-like behavior.
    auto snap = std::make_shared<state_t>(*state_);
    return std::static_pointer_cast<gsp::BaseMiniState>(snap);
}

// ---- dense path ----

template <class Matrix>
void StateMatrixGraph<Matrix>::reset_dense_() {
    state_->reset(is_directed_, num_nodes_);
    state_->col = is_directed_ ? 0u : state_->row; // undirected: start at diagonal
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next_dense_() {
    auto& st = *state_;

    while (st.row < num_nodes_) {
        while (st.col < num_nodes_) {
            const uint32_t r = st.row;
            const uint32_t c = st.col;

            const scalar_type w =
                (*weights_)(static_cast<Eigen::Index>(r), static_cast<Eigen::Index>(c));

            // Threshold
            if (std::abs(static_cast<double>(w)) <= thresh_) {
                ++st.col;
                continue;
            }

            // Undirected: keep only upper triangle
            if (!is_directed_ && r > c) {
                ++st.col;
                continue;
            }

            // Pin the edge location for subsequent setValue()
            st.last_row = r;
            st.last_col = c;

            // Edge owns a snapshot of the pinned state
            auto edge_state = snapshot_state_for_edge_();

            // Advance traversal state
            ++st.col;

            // NOTE: Edge now takes shared_ptr<BaseMiniState> as the last parameter.
            return gsp::Edge(r, c, static_cast<double>(w), edge_state);
        }

        ++st.row;
        st.col = is_directed_ ? 0u : st.row;
    }

    return std::nullopt;
}

// ---- sparse path ----

template <class Matrix>
void StateMatrixGraph<Matrix>::reset_sparse_() {
    state_->reset();
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next_sparse_() {
    static_assert(is_sparse_, "next_sparse_ only valid for sparse matrices.");

    auto& st = *state_;
    if (!st.weights) return std::nullopt;

    while (static_cast<int>(st.outer) < st.weights->outerSize()) {
        if (!st.it) {
            st.advance_to_next_nonempty_outer();
            if (!st.it) return std::nullopt;
        }

        if (!(*st.it)) {
            st.it.reset();
            ++st.outer;
            continue;
        }

        // Copy iterator for this element (so we can pin it as last_it)
        auto cur = *st.it;

        // Advance traversal iterator now
        ++(*st.it);

        const Eigen::Index r = cur.row();
        const Eigen::Index c = cur.col();
        const scalar_type  w = cur.value();

        // Undirected: keep only upper triangle
        if (!is_directed_ && r > c) {
            continue;
        }

        // Threshold
        if (std::abs(static_cast<double>(w)) <= thresh_) {
            continue;
        }

        // Pin the entry for Edge
        st.last_it = cur;

        // Snapshot pinned state for Edge ownership
        auto edge_state = snapshot_state_for_edge_();

        return gsp::Edge(static_cast<uint32_t>(r),
                         static_cast<uint32_t>(c),
                         static_cast<double>(w),
                         edge_state);
    }

    return std::nullopt;
}

} // namespace gsp

#endif // LIBGSP_STATEMATRIXGRAPH_H
