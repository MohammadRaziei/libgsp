//
// Created by Mohammad on 7/20/2025.
// (Generic scalar support; ONLY normalized_* are computed/stored in matrix_float_t)
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <optional>
#include <stdexcept>

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include "BaseGraph.h"
#include "libgsp/utils/Types.h"
#include "libgsp/utils/Logging.h"

namespace gsp {
namespace detail {
    template <class Matrix> class MatrixBox;
    template <class Matrix> class CacheBox;
}

class BaseGraph;
template <class Matrix> class Graph;

// NOTE: These aliases may still point to your default double types from Types.h,
// but Graph itself is now generic for any Eigen dense/sparse scalar.
using SparseGraph = Graph<sparsematrix>;
using DenseGraph  = Graph<densematrix>;

} // namespace gsp

// ============================================================
// gsp::Graph
// ============================================================

template <class Matrix>
class gsp::Graph : public gsp::BaseGraph {
public:
    using densevector = typename gsp::types::densevector_m<Matrix>;
    using value_type  = Matrix;

    // Normalized outputs are computed/stored in float_of<elem_t<Matrix>>
    using normalized_matrix = gsp::types::matrix_float_t<Matrix>;

public:
    explicit Graph(uint32_t num_nodes, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    Graph(const gsp::Graph<Matrix>& other) = delete;
    Graph(gsp::Graph<Matrix>&& other) noexcept;
    explicit Graph(const gsp::Graph<Matrix>* other) noexcept;
    explicit Graph(const gsp::VertexGraph* other) noexcept;
    explicit Graph(gsp::VertexGraph&& other) noexcept;

    ~Graph() override;

    void operator=(const gsp::Graph<Matrix>& other) = delete;
    gsp::Graph<Matrix>& operator=(gsp::Graph<Matrix>&& other) noexcept;

    void setEdges(const std::vector<gsp::Edge>& edges,
                  bool is_directed = GSP_IS_DIRECTED_DEFAULT) override;

    void setEdges(gsp::ConstEdgeGenerator& generator,
                  bool is_directed = GSP_IS_DIRECTED_DEFAULT);

    void setWeights(const Matrix& matrix, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    void setWeights(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT);

    virtual void validateWeights(const Matrix&);

    const Matrix& weights() const;
    const Matrix& laplacian();

    // ONLY these are float_of-based (matrix_float_t)
    const normalized_matrix& normalizedLaplacian();
    const normalized_matrix& normalizedWeight();
    const normalized_matrix& asymmetricNormalizedWeight();

    const densevector& degrees();

    // Edge iteration
    gsp::ConstEdgeGenerator iterEdges(double thresh = 0.0) const override;
    gsp::EdgeGenerator iterEdges(double thresh = 0.0) override;

    // Deep copy (preserves concrete Graph<Matrix>)
    gsp::Graph<Matrix> clone() const;

    template <class Target>
    gsp::Graph<Target> to(double thresh = 0.0) const {
        if constexpr (std::is_same_v<Matrix, Target>) {
            if (thresh == 0.0) {
                logger_->warn("Warning: Graph is already {}!", type_);
                return clone();
            }
        }
        Graph<Target> graph(dynamic_cast<const VertexGraph*>(this));
        auto gen = this->iterEdges(thresh);
        graph.setEdges(gen, this->is_directed_);
        return graph;
    }

    SparseGraph toSparse(double thresh = 0.0) const;
    DenseGraph  toDense(double thresh = 0.0) const;

    void invalidateCache();

    // operators:
    gsp::Graph<Matrix>& iadd(const gsp::Graph<Matrix>& other);
    gsp::Graph<Matrix>& operator+=(const gsp::Graph<Matrix>& other);
    gsp::Graph<Matrix> add(const gsp::Graph<Matrix>& other) const;
    gsp::Graph<Matrix> operator+(const gsp::Graph<Matrix>& other) const;

    gsp::Graph<Matrix> kron(const gsp::Graph<Matrix>& other) const;
    gsp::Graph<Matrix> mul(const gsp::Graph<Matrix>& other) const;
    gsp::Graph<Matrix> operator*(const gsp::Graph<Matrix>& other) const;

protected:
    Matrix weights_;
    gsp::logging::Logger logger_;

private:
    gsp::detail::CacheBox<Matrix>* cache();
    std::string detType() const;

    std::unique_ptr<gsp::detail::CacheBox<Matrix>> cache_;
};

// ============================================================
// Implementations
// ============================================================

#include "libgsp/utils/Matrix.h"
#include "libgsp/iterators/StateMatrixGraph.h"

namespace gsp::detail {

// ============================================================
// MatrixBox: caches derived matrices/vectors.
// - laplacian_ and degrees_ stay in original Matrix / densevector types.
// - normalized_* caches are matrix_float_t<Matrix>.
// ============================================================

template <typename Matrix, typename Weights>
static bool isMatrixBoxCalculated(const Matrix& m, Weights* weights) {
    return weights && (m.rows() == weights->rows()) && (m.cols() > 0);
}

template <class Matrix>
class MatrixBox {
public:
    using scalar_type = gsp::types::elem_t<Matrix>;
    using float_type  = gsp::types::float_of<scalar_type>;
    using float_matrix = gsp::types::matrix_float_t<Matrix>;
    using densevector  = typename gsp::Graph<Matrix>::densevector;

    MatrixBox() = default;
    MatrixBox(const MatrixBox&) = delete;
    MatrixBox& operator=(const MatrixBox&) = delete;

    explicit MatrixBox(const Matrix* weights);
    ~MatrixBox();

    void reset();
    void setWeights(const Matrix* weights);

    const Matrix& weights() const;

    // Base (kept in Matrix)
    Matrix& laplacian();
    densevector& degrees();

    // Normalized (computed/stored in float_matrix)
    float_matrix& normalizedWeight();
    float_matrix& normalizedLaplacian();
    float_matrix& asymmetricNormalizedWeight();

private:
    const Matrix* weights_ = nullptr;

    // Base caches (original types)
    Matrix laplacian_;
    densevector degrees_;

    // Normalized caches (float_of-based)
    float_matrix normalized_weights_;
    float_matrix normalized_laplacian_;
    float_matrix asymmetric_normalized_weight_;
};

template <class Matrix>
class CacheBox {
public:
    explicit CacheBox(gsp::Graph<Matrix>* graph);

    gsp::detail::MatrixBox<Matrix> matrix_;
};

} // namespace gsp::detail

// ---------------- Graph ----------------

template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes, bool is_directed)
    : BaseGraph(num_nodes),
      logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
    is_directed_ = is_directed;
}

template <class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::Graph<Matrix>* other) noexcept
    : gsp::BaseGraph(dynamic_cast<const BaseGraph*>(other)),
      weights_(other->weights_),
      logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}

template <class Matrix>
gsp::Graph<Matrix>::Graph(gsp::Graph<Matrix>&& other) noexcept
    : BaseGraph(std::move(other)),
      weights_(std::move(other.weights_)),
      logger_(gsp::logging::getLogger(detType())),
      cache_(std::move(other.cache_)) {
    type_ = detType();
}

template <class Matrix>
gsp::Graph<Matrix>::Graph(gsp::VertexGraph&& other) noexcept
    : BaseGraph(std::move(other)),
      logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}

template <class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::VertexGraph* other) noexcept
    : gsp::BaseGraph(other),
      logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}

template <class Matrix>
std::string gsp::Graph<Matrix>::detType() const {
    if constexpr (gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        return "SparseGraph";
    } else if constexpr (gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        return "DenseGraph";
    }
    return "UnknownGraph";
}

template <class Matrix>
gsp::Graph<Matrix>& gsp::Graph<Matrix>::operator=(gsp::Graph<Matrix>&& other) noexcept {
    if (this == &other) return *this;

    BaseGraph::operator=(std::move(other));
    weights_ = std::move(other.weights_);
    is_directed_ = other.is_directed_;
    cache_ = std::move(other.cache_);

    return *this;
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix, bool is_directed) {
    invalidateCache();
    weights_ = matrix;
    is_directed_ = is_directed;
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const std::vector<gsp::Edge>& edges, bool is_directed) {
    setEdges(edges, is_directed);
}

template <class Matrix>
void gsp::Graph<Matrix>::setEdges(const std::vector<gsp::Edge>& edges, bool is_directed) {
    invalidateCache();
    is_directed_ = is_directed;

    gsp::matrix::free(weights_);
    gsp::matrix::allocate(weights_, this->numNodes(), this->numNodes());
    gsp::matrix::fillZero(weights_);

    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight() == 0.0) continue;

        gsp::matrix::setElement(weights_, it->source(), it->target(), it->weight());

        if (!is_directed) {
            const double w_back = gsp::matrix::getElement(weights_, it->target(), it->source());
            if (w_back == 0.0) {
                gsp::matrix::setElement(weights_, it->target(), it->source(), it->weight());
            } else if (w_back != it->weight()) {
                gsp::matrix::free(weights_);
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            }
        }
    }
}

template <class Matrix>
void gsp::Graph<Matrix>::setEdges(gsp::ConstEdgeGenerator& generator, bool is_directed) {
    invalidateCache();
    is_directed_ = is_directed;

    gsp::matrix::free(weights_);
    gsp::matrix::allocate(weights_, this->numNodes(), this->numNodes());
    gsp::matrix::fillZero(weights_);

    while (auto it = generator.next()) {
        if (it->weight() == 0.0) continue;

        gsp::matrix::setElement(weights_, it->source(), it->target(), it->weight());

        if (!is_directed) {
            const double w_back = gsp::matrix::getElement(weights_, it->target(), it->source());
            if (w_back == 0.0) {
                gsp::matrix::setElement(weights_, it->target(), it->source(), it->weight());
            } else if (w_back != it->weight()) {
                gsp::matrix::free(weights_);
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            }
        }
    }
}

template <class Matrix>
void gsp::Graph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::Graph<Matrix>::~Graph() {
    gsp::matrix::free(weights_);
    invalidateCache();
}

template <class Matrix>
void gsp::Graph<Matrix>::invalidateCache() {
    cache_.reset();
}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::weights() const {
    return weights_;
}

template <class Matrix>
gsp::detail::CacheBox<Matrix>* gsp::Graph<Matrix>::cache() {
    if (!cache_) {
        cache_ = std::make_unique<gsp::detail::CacheBox<Matrix>>(this);
    }
    return cache_.get();
}

template <class Matrix>
gsp::ConstEdgeGenerator gsp::Graph<Matrix>::iterEdges(double thresh) const {
    auto state = std::make_shared<gsp::StateMatrixGraph<Matrix>>(
        const_cast<Matrix*>(&weights_), is_directed_, thresh);
    return gsp::ConstEdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator>(state));
}

template <class Matrix>
gsp::EdgeGenerator gsp::Graph<Matrix>::iterEdges(double thresh) {
    auto state = std::make_shared<gsp::StateMatrixGraph<Matrix>>(&weights_, is_directed_, thresh);
    return gsp::EdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator>(state));
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::clone() const {
    return gsp::Graph<Matrix>(this);
}

template <class Matrix>
gsp::SparseGraph gsp::Graph<Matrix>::toSparse(double thresh) const {
    return to<gsp::sparsematrix>(thresh);
}

template <class Matrix>
gsp::DenseGraph gsp::Graph<Matrix>::toDense(double thresh) const {
    return to<gsp::densematrix>(thresh);
}

// ---- cache-backed getters ----

template <class Matrix>
const typename gsp::Graph<Matrix>::densevector& gsp::Graph<Matrix>::degrees() {
    return cache()->matrix_.degrees();
}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::laplacian() {
    return cache()->matrix_.laplacian();
}

template <class Matrix>
const typename gsp::Graph<Matrix>::normalized_matrix& gsp::Graph<Matrix>::normalizedWeight() {
    return cache()->matrix_.normalizedWeight();
}

template <class Matrix>
const typename gsp::Graph<Matrix>::normalized_matrix& gsp::Graph<Matrix>::normalizedLaplacian() {
    return cache()->matrix_.normalizedLaplacian();
}

template <class Matrix>
const typename gsp::Graph<Matrix>::normalized_matrix& gsp::Graph<Matrix>::asymmetricNormalizedWeight() {
    return cache()->matrix_.asymmetricNormalizedWeight();
}

// ---------------- MatrixBox / CacheBox ----------------

template <class Matrix>
gsp::detail::MatrixBox<Matrix>::MatrixBox(const Matrix* matrix) : weights_(matrix) {}

template <class Matrix>
gsp::detail::MatrixBox<Matrix>::~MatrixBox() {
    reset();
}

template <class Matrix>
void gsp::detail::MatrixBox<Matrix>::reset() {
    weights_ = nullptr;

    gsp::matrix::free(laplacian_);
    degrees_.resize(0);

    // normalized_* are Eigen objects too, reuse your helper to free
    gsp::matrix::free(normalized_weights_);
    gsp::matrix::free(normalized_laplacian_);
    gsp::matrix::free(asymmetric_normalized_weight_);
}

template <class Matrix>
void gsp::detail::MatrixBox<Matrix>::setWeights(const Matrix* matrix) {
    reset();
    weights_ = matrix;
}

template <class Matrix>
const Matrix& gsp::detail::MatrixBox<Matrix>::weights() const {
    return *weights_;
}

template <class Matrix>
gsp::detail::CacheBox<Matrix>::CacheBox(gsp::Graph<Matrix>* graph)
    : matrix_(&graph->weights()) {}


// ---- degrees (kept in original densevector<Matrix>) ----

template <class Matrix>
typename gsp::detail::MatrixBox<Matrix>::densevector&
gsp::detail::MatrixBox<Matrix>::degrees() {
    if (isMatrixBoxCalculated(degrees_, weights_)) return degrees_;

    // NOTE: This keeps the original scalar. If scalar is integer, degrees_ will be integer.
    // Normalized methods will cast degrees_ to float_type as needed.
    degrees_ = (*weights_) * densevector::Ones(weights_->cols());
    return degrees_;
}

// ---- laplacian (kept in original Matrix) ----

template <class Matrix>
Matrix& gsp::detail::MatrixBox<Matrix>::laplacian() {
    if (isMatrixBoxCalculated(laplacian_, weights_)) return laplacian_;

    if (!weights_) throw std::runtime_error("MatrixBox::laplacian(): weights_ is null");

    auto& deg = degrees();

    if constexpr (gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        // L = D - W
        laplacian_ = -(*weights_);
        laplacian_.diagonal().array() += deg.array();
        return laplacian_;
    } else if constexpr (gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        // Build L = D - W (triplets)
        using triplet_t = Eigen::Triplet<scalar_type>;

        const int n = static_cast<int>(weights_->rows());
        std::vector<triplet_t> triplets;
        triplets.reserve(static_cast<size_t>(weights_->nonZeros()) + static_cast<size_t>(n));

        for (int k = 0; k < weights_->outerSize(); ++k) {
            for (typename std::remove_reference_t<Matrix>::InnerIterator it(*weights_, k); it; ++it) {
                if (it.row() != it.col()) {
                    triplets.emplace_back(it.row(), it.col(), static_cast<scalar_type>(-it.value()));
                }
            }
        }

        for (int i = 0; i < n; ++i) {
            const scalar_type diag =
                static_cast<scalar_type>(deg[i]) - static_cast<scalar_type>(weights_->coeff(i, i));
            triplets.emplace_back(i, i, diag);
        }

        laplacian_.resize(n, n);
        laplacian_.setFromTriplets(triplets.begin(), triplets.end());
        laplacian_.makeCompressed();
        return laplacian_;
    } else {
        static_assert(sizeof(Matrix) == 0, "MatrixBox<Matrix>::laplacian(): Matrix must be Eigen dense or sparse.");
    }
}

// ---- normalizedWeight (float_matrix) ----

template <class Matrix>
typename gsp::detail::MatrixBox<Matrix>::float_matrix&
gsp::detail::MatrixBox<Matrix>::normalizedWeight() {
    if (isMatrixBoxCalculated(normalized_weights_, weights_)) return normalized_weights_;
    if (!weights_) throw std::runtime_error("MatrixBox::normalizedWeight(): weights_ is null");

    // d^{-1/2}
    Eigen::Matrix<float_type, Eigen::Dynamic, 1> d_inv_sqrt =
        degrees().template cast<float_type>().unaryExpr([](float_type x) -> float_type {
            return (x > float_type(0)) ? (float_type(1) / std::sqrt(x)) : float_type(0);
        });

    if constexpr (gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        // D^{-1/2} W D^{-1/2}
        normalized_weights_ = weights_->template cast<float_type>();
        normalized_weights_ = normalized_weights_.array().rowwise() * d_inv_sqrt.transpose().array();
        normalized_weights_ = normalized_weights_.array().colwise() * d_inv_sqrt.array();
        return normalized_weights_;
    } else if constexpr (gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        normalized_weights_ = weights_->template cast<float_type>();

        for (int k = 0; k < normalized_weights_.outerSize(); ++k) {
            for (typename float_matrix::InnerIterator it(normalized_weights_, k); it; ++it) {
                it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
            }
        }
        return normalized_weights_;
    } else {
        static_assert(sizeof(Matrix) == 0, "MatrixBox<Matrix>::normalizedWeight(): Matrix must be Eigen dense or sparse.");
    }
}

// ---- asymmetricNormalizedWeight (float_matrix) ----

template <class Matrix>
typename gsp::detail::MatrixBox<Matrix>::float_matrix&
gsp::detail::MatrixBox<Matrix>::asymmetricNormalizedWeight() {
    if (isMatrixBoxCalculated(asymmetric_normalized_weight_, weights_)) return asymmetric_normalized_weight_;
    if (!weights_) throw std::runtime_error("MatrixBox::asymmetricNormalizedWeight(): weights_ is null");

    // d^{-1}
    Eigen::Matrix<float_type, Eigen::Dynamic, 1> d_inv =
        degrees().template cast<float_type>().unaryExpr([](float_type x) -> float_type {
            return (x > float_type(0)) ? (float_type(1) / x) : float_type(0);
        });

    if constexpr (gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        // D^{-1} W  (left scaling by d_inv on rows)
        asymmetric_normalized_weight_ = weights_->template cast<float_type>();
        asymmetric_normalized_weight_ = asymmetric_normalized_weight_.array().colwise() * d_inv.array();
        return asymmetric_normalized_weight_;
    } else if constexpr (gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        asymmetric_normalized_weight_ = weights_->template cast<float_type>();

        for (int k = 0; k < asymmetric_normalized_weight_.outerSize(); ++k) {
            for (typename float_matrix::InnerIterator it(asymmetric_normalized_weight_, k); it; ++it) {
                it.valueRef() *= d_inv[it.row()];
            }
        }
        return asymmetric_normalized_weight_;
    } else {
        static_assert(sizeof(Matrix) == 0, "MatrixBox<Matrix>::asymmetricNormalizedWeight(): Matrix must be Eigen dense or sparse.");
    }
}

// ---- normalizedLaplacian (float_matrix) ----

template <class Matrix>
typename gsp::detail::MatrixBox<Matrix>::float_matrix&
gsp::detail::MatrixBox<Matrix>::normalizedLaplacian() {
    if (isMatrixBoxCalculated(normalized_laplacian_, weights_)) return normalized_laplacian_;
    if (!weights_) throw std::runtime_error("MatrixBox::normalizedLaplacian(): weights_ is null");

    // d^{-1/2}
    Eigen::Matrix<float_type, Eigen::Dynamic, 1> d_inv_sqrt =
        degrees().template cast<float_type>().unaryExpr([](float_type x) -> float_type {
            return (x > float_type(0)) ? (float_type(1) / std::sqrt(x)) : float_type(0);
        });

    if constexpr (gsp::types::is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        // D^{-1/2} L D^{-1/2}
        normalized_laplacian_ = laplacian().template cast<float_type>();
        normalized_laplacian_ = normalized_laplacian_.array().rowwise() * d_inv_sqrt.transpose().array();
        normalized_laplacian_ = normalized_laplacian_.array().colwise() * d_inv_sqrt.array();
        return normalized_laplacian_;
    } else if constexpr (gsp::types::is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value) {
        normalized_laplacian_ = laplacian().template cast<float_type>();

        for (int k = 0; k < normalized_laplacian_.outerSize(); ++k) {
            for (typename float_matrix::InnerIterator it(normalized_laplacian_, k); it; ++it) {
                it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
            }
        }
        return normalized_laplacian_;
    } else {
        static_assert(sizeof(Matrix) == 0, "MatrixBox<Matrix>::normalizedLaplacian(): Matrix must be Eigen dense or sparse.");
    }
}

// ---------------- operators ----------------

template <class Matrix>
gsp::Graph<Matrix>& gsp::Graph<Matrix>::iadd(const gsp::Graph<Matrix>& other) {
    if (numNodes() != other.numNodes()) {
        std::string msg = fmt::format("Number of nodes has mismatched! ({} != {})", numNodes(), other.numNodes());
        logger_->error(msg);
        throw std::invalid_argument(msg);
    }
    gsp::VertexGraph::iadd(dynamic_cast<const gsp::VertexGraph&>(other));
    is_directed_ |= other.is_directed_;
    weights_ += other.weights_;
    invalidateCache();
    return *this;
}

template <class Matrix>
gsp::Graph<Matrix>& gsp::Graph<Matrix>::operator+=(const gsp::Graph<Matrix>& other) {
    return iadd(other);
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::add(const gsp::Graph<Matrix>& other) const {
    gsp::Graph<Matrix> out(this);
    out.iadd(other);
    return out;
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::operator+(const gsp::Graph<Matrix>& other) const {
    return add(other);
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::operator*(const gsp::Graph<Matrix>& other) const {
    return mul(other);
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::kron(const gsp::Graph<Matrix>& other) const {
    gsp::Graph<Matrix> graph(std::move(gsp::VertexGraph::mul(other)));
    bool is_directed = is_directed_ || other.is_directed_;
    auto w = gsp::matrix::kron(weights_, other.weights_);
    graph.setWeights(w, is_directed);
    return graph;
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::mul(const gsp::Graph<Matrix>& other) const {
    gsp::Graph<Matrix> graph(std::move(gsp::VertexGraph::mul(other)));
    bool is_directed = is_directed_ || other.is_directed_;
    auto w = gsp::matrix::kronSum(weights_, other.weights_);
    graph.setWeights(w, is_directed);
    return graph;
}

#endif  // LIBGSP_GRAPH_H
