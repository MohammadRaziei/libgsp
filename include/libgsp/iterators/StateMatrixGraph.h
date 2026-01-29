//
// Created by mohammad on 1/10/26.
//

#ifndef LIBGSP_STATEMATRIXGRAPH_H
#define LIBGSP_STATEMATRIXGRAPH_H
#pragma once

#include "libgsp/iterators/EdgeGenerator.h"
#include "libgsp/utils/Types.h"

#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include <cmath>
#include <memory>
#include <optional>

namespace gsp {
    template <class Matrix> class StateMatrixGraph;
}

template <class Matrix>
class gsp::StateMatrixGraph : public gsp::BaseStateEdgeGenerator {
public:
    StateMatrixGraph(Matrix* weight, bool is_directed, double thresh);
    StateMatrixGraph(const StateMatrixGraph& other) = delete;
    StateMatrixGraph(const StateMatrixGraph* other);
    StateMatrixGraph& operator=(const StateMatrixGraph&) = delete;
    ~StateMatrixGraph();

    virtual void reset() override;
    virtual std::optional<gsp::Edge> next() override;
    virtual std::shared_ptr<gsp::BaseStateEdgeGenerator> clone() const override;
    virtual void setWeight(double weight);


private:
        // common state (used by specializations)
        Matrix* weights_ = nullptr;
        uint32_t num_nodes_;
        double thresh_;
        bool is_directed_;

        class State;
        std::unique_ptr<State> state_;  // Using PIMPL pattern for state
};



namespace gsp {

// Dense matrix state implementation
template <>
class StateMatrixGraph<gsp::densematrix>::State {
public:
    State(const gsp::densematrix*) { reset(); }
    State(const gsp::StateMatrixGraph<gsp::densematrix>::State& other) :
        row_(other.row_), col_(other.col_) {}

    void reset() { row_ = col_ = 0; }
    uint32_t row_, col_;
};

// Sparse matrix state implementation
template <>
class StateMatrixGraph<gsp::sparsematrix>::State {
public:
    using InnerIt = gsp::sparsematrix::InnerIterator;

    explicit State(gsp::sparsematrix* W) : W(W) { reset(); }
    State(const gsp::StateMatrixGraph<gsp::sparsematrix>::State& other) :
        W(other.W), outer(other.outer), it(std::make_unique<InnerIt>(*other.it.get())) {}

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

    sparsematrix* W = nullptr;
    uint32_t outer = 0;                          // current row
    std::unique_ptr<InnerIt> it;            // iterator within current row
};

// Constructor with threshold
template <class Matrix>
StateMatrixGraph<Matrix>::StateMatrixGraph(Matrix* weights, bool is_directed, double thresh)
    : gsp::BaseStateEdgeGenerator(), weights_(weights), num_nodes_(weights->rows()), is_directed_(is_directed), thresh_(thresh) {
        state_ = std::make_unique<State>(weights_);
    state_->reset();   // Reset the state
}

template <class Matrix>
StateMatrixGraph<Matrix>::StateMatrixGraph(const StateMatrixGraph* other): gsp::BaseStateEdgeGenerator(),
                                                                           weights_(other->weights_), num_nodes_(other->num_nodes_), is_directed_(other->is_directed_), thresh_(other->thresh_){
    auto st = other->state_.get();
    state_ = std::make_unique<State>(*st);
}

// Destructor
template <class Matrix>
StateMatrixGraph<Matrix>::~StateMatrixGraph() = default;

// Reset (keeps current threshold)
template <class Matrix>
void StateMatrixGraph<Matrix>::reset() {
    state_->reset();
}

template <>
void StateMatrixGraph<gsp::sparsematrix>::setWeight(double weight) {
    auto* st = state_.get();
    st->it->valueRef() = weight;
}

template <>
void StateMatrixGraph<gsp::densematrix>::setWeight(double weight) {
    auto* st = state_.get();
    (*weights_)(st->row_, st->col_) = weight;
}


// Next edge for dense matrix
template <>
std::optional<Edge> StateMatrixGraph<densematrix>::next() {
    if (!weights_ || num_nodes_ <= 0 || !state_ || weights_->rows() == 0) return std::nullopt;

    while (state_->row_ < num_nodes_) {
        // for undirected we emit only upper triangle: col starts at row
        while (state_->col_ < num_nodes_) {
            const uint32_t col = state_->col_;
            const double w = (*weights_)(state_->row_, col);

            if (std::abs(w) <= thresh_) {
                ++state_->col_;
                continue;
            }
            auto cloned = clone();
            ++state_->col_;

            return gsp::Edge(state_->row_, col, w, cloned);
        }
        ++state_->row_;
        state_->col_ = (is_directed_) ? 0 : state_->row_; // reset for next row
    }
    return std::nullopt;
}

// Next edge for sparse matrix
template <>
std::optional<Edge> StateMatrixGraph<sparsematrix>::next() {
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

        // undirected: keep only upper triangle (i <= j)
        if ((!is_directed_ && r > c) || (std::abs(w) <= static_cast<double>(thresh_))) {
            ++(*st->it);
            continue;
        }
        auto cloned = clone();
        ++(*st->it);

        return gsp::Edge(static_cast<uint32_t>(r),
                    static_cast<uint32_t>(c),
                    w, cloned);
    }

    return std::nullopt;
}


template<class Matrix>
std::shared_ptr<gsp::BaseStateEdgeGenerator> gsp::StateMatrixGraph<Matrix>::clone() const {
    gsp::BaseStateEdgeGenerator* st = new gsp::StateMatrixGraph<Matrix>(this);
    return std::shared_ptr<gsp::BaseStateEdgeGenerator>(st);
}

// Explicit instantiations
template class StateMatrixGraph<densematrix>;
template class StateMatrixGraph<sparsematrix>;

} // namespace gsp


#endif 