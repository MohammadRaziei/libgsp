//
// gsp::StateMatrixGraph — generic Eigen dense/sparse edge generator
// - Works with Eigen::Matrix<Scalar,...> and Eigen::SparseMatrix<Scalar,...>
// - Prototypes first, implementations below
// - Member variables are snake_case
// - thresh_ stays double
// - Uses gsp::types::{elem_t,is_eigen_dense,is_eigen_sparse,float_of}
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
    std::shared_ptr<gsp::BaseStateEdgeGenerator> clone() const override;
    void setWeight(double weight) override;

private:
    // ---- matrix category ----
    static constexpr bool is_dense_ =
        gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value;

    static constexpr bool is_sparse_ =
        gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value;

    static_assert(is_dense_ || is_sparse_,
                  "StateMatrixGraph<Matrix>: Matrix must be an Eigen dense Matrix or Eigen SparseMatrix.");

    // ---- state ----
    struct dense_state {
        uint32_t row = 0;
        uint32_t col = 0;
        uint32_t last_row = 0; // last emitted edge position (for setWeight)
        uint32_t last_col = 0;
    };

    struct sparse_state {
        using sparse_matrix  = std::remove_cv_t<std::remove_reference_t<Matrix>>;
        using inner_iterator = typename sparse_matrix::InnerIterator;

        Matrix* weights = nullptr;
        uint32_t outer = 0;

        // Current iterator for current outer (RowMajor: row)
        std::optional<inner_iterator> it;

        // Last emitted iterator (for setWeight)
        std::optional<inner_iterator> last_it;
    };

    using state_t = std::conditional_t<is_dense_, dense_state, sparse_state>;

    // ---- data members (snake_case) ----
    Matrix*  weights_     = nullptr;
    uint32_t num_nodes_   = 0;
    double   thresh_      = 0.0;
    bool     is_directed_ = false;
    state_t  state_;

private:
    // ---- helpers ----
    // Dense
    void reset_dense_();
    std::optional<gsp::Edge> next_dense_();

    // Sparse
    void reset_sparse_();
    void advance_to_next_nonempty_outer_();
    std::optional<gsp::Edge> next_sparse_();
};

} // namespace gsp

// ============================
// Implementations
// ============================

namespace gsp {

template <class Matrix>
StateMatrixGraph<Matrix>::StateMatrixGraph(Matrix* weights, bool is_directed, double thresh)
    : weights_(weights),
      num_nodes_(weights ? static_cast<uint32_t>(weights->rows()) : 0),
      thresh_(thresh),
      is_directed_(is_directed),
      state_{} {
    reset();
}

template <class Matrix>
void StateMatrixGraph<Matrix>::reset() {
    if constexpr (is_dense_) {
        reset_dense_();
    } else {
        reset_sparse_();
    }
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next() {
    if (!weights_ || num_nodes_ == 0 || weights_->rows() == 0) return std::nullopt;

    if constexpr (is_dense_) {
        return next_dense_();
    } else {
        return next_sparse_();
    }
}

template <class Matrix>
std::shared_ptr<gsp::BaseStateEdgeGenerator> StateMatrixGraph<Matrix>::clone() const {
    // NOTE: clone shares weights pointer and copies iteration state
    auto p = std::make_shared<StateMatrixGraph<Matrix>>(weights_, is_directed_, thresh_);
    p->num_nodes_ = num_nodes_;
    p->state_ = state_;
    return std::static_pointer_cast<gsp::BaseStateEdgeGenerator>(p);
}

template <class Matrix>
void StateMatrixGraph<Matrix>::setWeight(double weight) {
    if (!weights_) return;

    if constexpr (is_dense_) {
        (*weights_)(static_cast<Eigen::Index>(state_.last_row),
                    static_cast<Eigen::Index>(state_.last_col)) =
            static_cast<scalar_type>(static_cast<float_type>(weight));
    } else {
        if (state_.last_it) {
            state_.last_it->valueRef() =
                static_cast<scalar_type>(static_cast<float_type>(weight));
        }
    }
}

// --------------------------
// Dense implementation
// --------------------------

template <class Matrix>
void StateMatrixGraph<Matrix>::reset_dense_() {
    state_.row = 0;
    state_.col = is_directed_ ? 0u : 0u;
    state_.last_row = 0;
    state_.last_col = 0;
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next_dense_() {
    while (state_.row < num_nodes_) {
        while (state_.col < num_nodes_) {
            const uint32_t r = state_.row;
            const uint32_t c = state_.col;

            const scalar_type w = (*weights_)(static_cast<Eigen::Index>(r),
                                             static_cast<Eigen::Index>(c));

            // Threshold check (promote to float_of<scalar> then compare in double space)
            if (std::abs(static_cast<double>(w)) <= thresh_) {
                ++state_.col;
                continue;
            }

            // Undirected: keep only upper triangle
            if (!is_directed_ && r > c) {
                ++state_.col;
                continue;
            }

            // Record last emitted position for setWeight()
            state_.last_row = r;
            state_.last_col = c;

            auto cloned = clone();
            ++state_.col;

            return gsp::Edge(r, c, static_cast<double>(static_cast<float_type>(w)), cloned);
        }

        ++state_.row;
        state_.col = is_directed_ ? 0u : state_.row;
    }

    return std::nullopt;
}

// --------------------------
// Sparse implementation
// --------------------------

template <class Matrix>
void StateMatrixGraph<Matrix>::reset_sparse_() {
    state_.weights = weights_;
    state_.outer = 0;
    state_.it.reset();
    state_.last_it.reset();
    advance_to_next_nonempty_outer_();
}

template <class Matrix>
void StateMatrixGraph<Matrix>::advance_to_next_nonempty_outer_() {
    static_assert(is_sparse_, "advance_to_next_nonempty_outer_ only valid for sparse matrices.");
    if (!state_.weights) return;

    const int outer_size = state_.weights->outerSize(); // RowMajor: rows
    while (static_cast<int>(state_.outer) < outer_size) {
        state_.it.emplace(*state_.weights, static_cast<int>(state_.outer));
        if (*state_.it) return; // found non-empty outer
        state_.it.reset();
        ++state_.outer;
    }
}

template <class Matrix>
std::optional<gsp::Edge> StateMatrixGraph<Matrix>::next_sparse_() {
    static_assert(is_sparse_, "next_sparse_ only valid for sparse matrices.");
    if (!state_.weights) return std::nullopt;

    while (static_cast<int>(state_.outer) < state_.weights->outerSize()) {
        if (!state_.it) {
            advance_to_next_nonempty_outer_();
            if (!state_.it) return std::nullopt;
        }

        if (!(*state_.it)) {
            // current outer exhausted
            state_.it.reset();
            ++state_.outer;
            continue;
        }

        // Keep a copy pointing to current element for last_it/setWeight
        auto cur = *state_.it;

        // Advance the live iterator now
        ++(*state_.it);

        const Eigen::Index r = cur.row();
        const Eigen::Index c = cur.col();
        const scalar_type  w = cur.value();

        // Undirected: keep only upper triangle
        if (!is_directed_ && r > c) {
            continue;
        }

        // Threshold check
        if (std::abs(static_cast<double>(w)) <= thresh_) {
            continue;
        }

        state_.last_it = cur;
        auto cloned = clone();

        return gsp::Edge(static_cast<uint32_t>(r),
                         static_cast<uint32_t>(c),
                         static_cast<double>(static_cast<float_type>(w)),
                         cloned);
    }

    return std::nullopt;
}

} // namespace gsp

#endif // LIBGSP_STATEMATRIXGRAPH_H
