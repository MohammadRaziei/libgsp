//
// Created by Mohammad on 7/20/2025.
//

#include <ciso646>
#include <type_traits>
#include <iostream>

#include "libgsp/Graph.h"
#include "libgsp/utils/Matrix.h"
#include "libgsp/iterators/StateGraph.h"

template <class Matrix>
class gsp::MatrixBox {
   public:
    MatrixBox() = default;
    MatrixBox(const MatrixBox&) = delete;
    MatrixBox& operator=(const MatrixBox&) = delete;

    explicit MatrixBox(const Matrix* weights);
    ~MatrixBox();

    void reset();

    void setWeights(Matrix* weights);

    const Matrix& weights() const;
    Matrix& normalizedWeight();
    Matrix& laplacian();
    Matrix& normalizedLaplacian();
    typename gsp::Graph<Matrix>::densevector& degrees();
   private:
    bool isCalculated(const Matrix&);
    bool isCalculated(const typename gsp::Graph<Matrix>::densevector&);

    const Matrix* weights_;
    Matrix laplacian_, normalized_weights_, normalized_laplacian_;
    typename gsp::Graph<Matrix>::densevector _degrees;
};

template <class Matrix>
class gsp::CacheBox {
   public:
    CacheBox(gsp::Graph<Matrix>* graph);

    gsp::MatrixBox<Matrix> matrix_;
};


template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes):
      BaseGraph(num_nodes), is_directed_(GSP_IS_DIRECTED_DEFAULT),
      logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::Graph<Matrix> *other) noexcept :
    gsp::BaseGraph(dynamic_cast<const BaseGraph*>(other)),
    is_directed_(other->is_directed_), weights_(other->weights_),
    logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(gsp::Graph<Matrix> &&other) noexcept
    : BaseGraph(std::move(other)), weights_(std::move(other.weights_)),
      is_directed_(other.is_directed_), cache_(other.cache_),
      logger_(gsp::logging::getLogger(detType())) {
    // Reset the moved-from object's cache pointer
    other.cache_ = nullptr;
    type_ = detType();
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::VertexGraph *other) noexcept: gsp::BaseGraph(other),
        logger_(gsp::logging::getLogger(detType())) {
    type_ = detType();
}

template<class Matrix>
std::string gsp::Graph<Matrix>::detType() const {
    if constexpr (gsp::types::is_eigen_sparse<Matrix>::value) {
        return "SparseGraph";
    } else if constexpr  (gsp::types::is_eigen_dense<Matrix>::value)  {
        return "DenseGraph";
    } else {
        return "UnknownGraph";
    }
}

template<class Matrix>
gsp::Graph<Matrix>& gsp::Graph<Matrix>::operator=(gsp::Graph<Matrix> &&other) noexcept {
    if (this == &other) return *this;

    // Call base class move assignment operator
    BaseGraph::operator=(std::move(other));

    // Move the matrix
    weights_ = std::move(other.weights_);

    // Move other members
    is_directed_ = other.is_directed_;

    // Move the cache
    delete cache_;  // Clean up existing cache
    cache_ = other.cache_;
    other.cache_ = nullptr;  // Reset moved-from object's cache pointer

    return *this;
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix, bool is_directed){
    invalidateCache();
    this->weights_ = matrix;
    this->is_directed_ = is_directed;
}


template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const std::vector<gsp::Edge>& edges, bool is_directed) {
    setEdges(edges, is_directed);
}

template <class Matrix>
void gsp::Graph<Matrix>::setEdges(const std::vector<gsp::Edge>& edges, bool is_directed) {
    invalidateCache();
    this->is_directed_ = is_directed;
    gsp::matrix::free(this->weights_);
    gsp::matrix::allocate(this->weights_, this->numNodes(), this->numNodes());
    gsp::matrix::fillZero(this->weights_);
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight() == 0) continue;
        gsp::matrix::setElement(this->weights_, it->source(), it->target(), it->weight());
        if (!is_directed) {
            const double w = gsp::matrix::getElement(this->weights_, it->target(), it->source());
            if (w == 0) {
                gsp::matrix::setElement(this->weights_, it->target(), it->source(), it->weight());
            } else if (w != it->weight()) {
                gsp::matrix::free(this->weights_);
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            }
        }
    }
}

template <class Matrix>
void gsp::Graph<Matrix>::setEdges(gsp::ConstEdgeGenerator& generator, bool is_directed) {
    invalidateCache();
    this->is_directed_ = is_directed;
    gsp::matrix::free(this->weights_);
    gsp::matrix::allocate(this->weights_, this->numNodes(), this->numNodes());
    gsp::matrix::fillZero(this->weights_);
    while (auto it = generator.next()) {
        if (it->weight() == 0) continue;
        gsp::matrix::setElement(this->weights_, it->source(), it->target(), it->weight());
        if (!is_directed) {
            const double w = gsp::matrix::getElement(this->weights_, it->target(), it->source());
            if (w == 0) {
                gsp::matrix::setElement(this->weights_, it->target(), it->source(), it->weight());
            } else if (w != it->weight()) {
                gsp::matrix::free(this->weights_);
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            }
        }
    }
}



template <class Matrix>
bool gsp::Graph<Matrix>::isDirected() const {
    return this->is_directed_;
}

template <class Matrix>
void gsp::Graph<Matrix>::setIsDirectedUnsafe(bool is_directed) {
    this->is_directed_ = is_directed;
}




template <class Matrix>
void gsp::Graph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::Graph<Matrix>::~Graph() {
    gsp::matrix::free(this->weights_);
    invalidateCache();
}

template <class Matrix>
void gsp::Graph<Matrix>::invalidateCache() {
    delete this->cache_;
    this->cache_ = nullptr;
}


template <class Matrix>
gsp::ConstEdgeGenerator gsp::Graph<Matrix>::iterEdges(double thresh) const {
    BaseStateEdgeGenerator* state = new gsp::StateGraph<Matrix>(const_cast<Matrix*>(&weights_), numNodes(), is_directed_, thresh);
    return gsp::ConstEdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator>(state));
}

template <class Matrix>
gsp::EdgeGenerator gsp::Graph<Matrix>::iterEdges(double thresh) {
    BaseStateEdgeGenerator* state = new gsp::StateGraph<Matrix>(&weights_, numNodes(), is_directed_, thresh);
    return gsp::EdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator>(state));
}

template <class Matrix>
gsp::Graph<Matrix> gsp::Graph<Matrix>::clone() const {
    return gsp::Graph<Matrix>(this);
}

// Implementation of toSparse method
template <class Matrix>
gsp::SparseGraph gsp::Graph<Matrix>::toSparse(double thresh) const {
    return to<gsp::sparsematrix>(thresh);
}

// Implementation of toDense method
template <typename Matrix>
gsp::DenseGraph gsp::Graph<Matrix>::toDense(double thresh) const {
    return to<gsp::densematrix>(thresh);
}
















template <class Matrix>
gsp::MatrixBox<Matrix>::MatrixBox(const Matrix* matrix) : weights_(matrix) {
}

template <class Matrix>
gsp::MatrixBox<Matrix>::~MatrixBox() {
    reset();
}


template <class Matrix>
void gsp::MatrixBox<Matrix>::reset() {
    this->weights_ = nullptr;
    gsp::matrix::free(this->laplacian_);
    gsp::matrix::free(this->normalized_laplacian_);
    gsp::matrix::free(this->normalized_weights_);
}

template <class Matrix>
void gsp::MatrixBox<Matrix>::setWeights(Matrix* matrix) {
    reset();
    this->weights_ = matrix;
}

template <class Matrix>
const Matrix& gsp::MatrixBox<Matrix>::weights() const {
    return *(this->weights_);
}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::weights() const {
    return this->weights_;
}

template <class Matrix>
gsp::CacheBox<Matrix>* gsp::Graph<Matrix>::cache() {
    if (not cache_) {
        cache_ = new gsp::CacheBox<Matrix>(this);
    }
    return cache_;
}


template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::laplacian() {
    if (isCalculated(this->laplacian_)) {
        return this->laplacian_;
    }
    // degree[i] = sum_j W(i,j)
    auto& cached_degrees = this->degrees();

    // L = D - W (dense, no extra temporaries)
    laplacian_ = -*weights_;
    laplacian_.diagonal().array() += cached_degrees.array();

    return laplacian_;
}

template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::laplacian() {
    if (isCalculated(this->laplacian_)) {
        return this->laplacian_; // already computed
    }

    const auto n = static_cast<int>(weights_->rows());

    // degree[i] = sum_j W(i,j)
    auto& cached_degrees = this->degrees();

    // Build L = D - W using triplets
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(static_cast<size_t>(weights_->nonZeros()) + n);

    // Off-diagonal: -W(i,j)
    for (int k = 0; k < weights_->outerSize(); ++k) {
        for (typename gsp::sparsematrix::InnerIterator it(*weights_, k); it; ++it) {
            if (it.row() != it.col()) {
                triplets.emplace_back(it.row(), it.col(), -it.value());
            }
        }
    }
    // Diagonal: degree[i] - W(i,i)  (keeps self-loop logic correct)
    for (int i = 0; i < n; ++i) {
        triplets.emplace_back(i, i, cached_degrees[i] - weights_->coeff(i, i));
    }

    laplacian_.resize(n, n);
    laplacian_.setFromTriplets(triplets.begin(), triplets.end());
    laplacian_.makeCompressed();

    return laplacian_;
}


template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::normalizedWeight() {
    if (isCalculated(this->normalized_weights_)) {
        return this->normalized_weights_; // already computed
    }
    auto& cached_degrees = this->degrees();


    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });


    this->normalized_weights_ = weights_->array().rowwise() * d_inv_sqrt.transpose().array(); // right scaling (columns)
    this->normalized_weights_ = this->normalized_weights_.array().colwise() * d_inv_sqrt.array();  // left  scaling (rows)

    return normalized_weights_;
}


template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::normalizedWeight() {
    if (isCalculated(this->normalized_weights_)) {
        return this->normalized_weights_; // already computed
    }

    auto& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    this->normalized_weights_ = *weights_;

    using SparseT = std::remove_reference_t<decltype(normalized_weights_)>;

    for (int k = 0; k < this->normalized_weights_.outerSize(); ++k) {
        for (SparseT::InnerIterator it(normalized_weights_, k); it; ++it) {
            it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
        }
    }

    return normalized_weights_;
}




template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::normalizedLaplacian() {
    if (isCalculated(this->normalized_laplacian_)) {
        return this->normalized_laplacian_;
    }

    auto& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0 / std::sqrt(x) : 0.0; });

    const auto& cached_laplacian = this->laplacian();

    this->normalized_laplacian_ = cached_laplacian.array().rowwise() * d_inv_sqrt.transpose().array(); // right scaling (columns)
    this->normalized_laplacian_ = this->normalized_laplacian_.array().colwise() * d_inv_sqrt.array();  // left  scaling (rows)



    return laplacian_;
}

template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::normalizedLaplacian() {
    if (isCalculated(this->normalized_laplacian_)) {
        return this->normalized_laplacian_; // already computed
    }

    const auto n = static_cast<int>(weights_->rows());

    auto& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    auto& cached_laplacian = this->laplacian();

    this->normalized_laplacian_.swap(cached_laplacian);

    using SparseT = std::remove_reference_t<decltype(normalized_laplacian_)>;

    for (int k = 0; k < this->normalized_laplacian_.outerSize(); ++k) {
        for (SparseT::InnerIterator it(normalized_laplacian_, k); it; ++it) {
            it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
        }
    }

    return normalized_laplacian_;
}

template <class Matrix>
const typename gsp::Graph<Matrix>::densevector& gsp::Graph<Matrix>::degrees() {
    return this->cache()->matrix_.degrees();
}



template <class Matrix>
const Matrix& gsp::Graph<Matrix>::laplacian() {
    return this->cache()->matrix_.laplacian();

}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::normalizedLaplacian() {
    return this->cache()->matrix_.normalizedLaplacian();

}


template <class Matrix>
typename gsp::Graph<Matrix>::densevector& gsp::MatrixBox<Matrix>::degrees() {
    if (isCalculated(this->_degrees)) {
        return _degrees;
    }
    _degrees = *weights_ * gsp::Graph<Matrix>::densevector::Ones(weights_->cols());
    return _degrees;
}



template <class Matrix>
gsp::CacheBox<Matrix>::CacheBox(gsp::Graph<Matrix>* graph) : matrix_(&graph->weights()) {

}


template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const Matrix& m) {
    return m.rows() == weights_->rows();
}
template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const typename gsp::Graph<Matrix>::densevector& m) {
    return m.rows() == weights_->rows();
}



template class gsp::Graph<gsp::densematrix>;  /// DenseMatrix
template class gsp::Graph<gsp::sparsematrix>; /// SparseMatrix

template class gsp::MatrixBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::MatrixBox<gsp::sparsematrix>; /// SparseMatrix

template class gsp::CacheBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::CacheBox<gsp::sparsematrix>; /// SparseMatrix




