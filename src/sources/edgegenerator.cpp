//
// Created by mohammad on 8/12/25.
//

#include "libgsp/graph/edgegenerator.h"
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/SparseCore>

namespace gsp {

// ===================== densematrix =====================
template <>
EdgeGenerator<densematrix>::EdgeGenerator(const gsp::Graph<densematrix>* graph, double thresh) {
    G_        = graph;
    W_        = graph ? &graph->weights() : nullptr;
    n_        = graph ? static_cast<int>(graph->num_nodes) : 0;
    thr_      = thresh;
    directed_ = graph ? graph->isDirected() : false;
    i_ = j_   = 0;
}


template <class Matrix>
EdgeGenerator<Matrix>::~EdgeGenerator() = default;

template <>
void EdgeGenerator<densematrix>::iter() {
    i_ = 0; j_ = 0;
}

template <>
void EdgeGenerator<sparsematrix>::iter() {
    outer_ = 0;
    k_ = (W_ && W_->outerSize() > 0) ? outerPtr_[0] : 0;
}

template <>
std::optional<Edge> EdgeGenerator<densematrix>::next() {
    if (!W_ || n_ <= 0) return std::nullopt;

    while (i_ < n_) {
        while (j_ < n_) {
            const int ii = i_;
            const int jj = j_++;
            if (!directed_ && ii > jj) continue;     // undirected: only i <= j
            const double w = (*W_)(ii, jj);
            if (std::abs(w) <= thr_) continue;

            return Edge(static_cast<uint32_t>(ii),
                        static_cast<uint32_t>(jj),
                        w);
        }
        ++i_;
        j_ = 0;
    }
    return std::nullopt;
}

// ===================== sparsematrix (RowMajor) =====================
template <>
EdgeGenerator<sparsematrix>::EdgeGenerator(const gsp::Graph<sparsematrix>* graph, double thresh) {
    G_        = graph;
    W_        = graph ? &graph->weights() : nullptr;
    n_        = graph ? static_cast<int>(graph->num_nodes) : 0;
    thr_      = thresh;
    directed_ = graph ? graph->isDirected() : false;

    outer_ = 0; k_ = 0;
    outerPtr_ = innerIdx_ = nullptr;
    values_   = nullptr;

    if (W_) {
        outerPtr_ = W_->outerIndexPtr(); // RowMajor: outer=row
        innerIdx_ = W_->innerIndexPtr(); // inner=col
        values_   = W_->valuePtr();
        k_        = (W_->outerSize() > 0) ? outerPtr_[0] : 0;
    }
}




template <>
std::optional<Edge> EdgeGenerator<sparsematrix>::next() {
    if (!W_ || n_ <= 0) return std::nullopt;

    // RowMajor compressed rows: outer=row, innerIdx_[k]=col
    while (outer_ < W_->outerSize()) {
        const int kend = outerPtr_[outer_ + 1];
        while (k_ < kend) {
            const int kk  = k_++;
            const int row = outer_;
            const int col = innerIdx_[kk];
            const double w = values_[kk];

            if (!directed_ && row > col) continue;   // undirected: only i <= j
            if (std::abs(w) <= thr_)       continue;

            return Edge(static_cast<uint32_t>(row),
                        static_cast<uint32_t>(col),
                        w);
        }
        ++outer_;
        if (outer_ < W_->outerSize()) k_ = outerPtr_[outer_];
    }
    return std::nullopt;
}


template <class Matrix>
std::vector<Edge> EdgeGenerator<Matrix>::toVector() {
    std::vector<Edge> edges;
    const size_t max_num = (directed_) ? n_ * n_ : n_ * (n_ + 1) / 2; // max edges in undirected graph
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
