//
// Created by Mohammad on 7/20/2025.
//
#include  <Eigen/Eigen>

#include "libgsp/graph/graph.h"
#include "libgsp/utils/matrix.h"


template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes):
      BaseGraph(num_nodes) {
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix, bool is_directed){
    invalidateCache();
    this->_weights = matrix;
    this->_is_directed = is_directed;
}


template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const std::vector<gsp::Edge>& edges, bool is_directed) {
    invalidateCache();
    this->_is_directed = is_directed;
    gsp::matrix::free(this->_weights);
    gsp::matrix::allocate(this->_weights, this->num_nodes, this->num_nodes);
    gsp::matrix::fillZero(this->_weights);
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight == 0) continue;
        gsp::matrix::setElement(this->_weights, it->source, it->target, it->weight);
        if (!is_directed) {
            double w = gsp::matrix::getElement(this->_weights, it->target, it->source);
            if (w == 0) {
                gsp::matrix::setElement(this->_weights, it->target, it->source, it->weight);
            } else if (w != it->weight) {
                gsp::matrix::free(this->_weights);
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            }
        }
    }
}


template <class Matrix>
bool gsp::Graph<Matrix>::isDirected() const {
    return this->_is_directed;
}




template <class Matrix>
void gsp::Graph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::Graph<Matrix>::~Graph() {
    gsp::matrix::free(this->_weights);
    invalidateCache();
}

template <class Matrix>
void gsp::Graph<Matrix>::invalidateCache() {
    delete this->_cache;
    this->_cache = nullptr;
}



template <class Matrix>
void gsp::Graph<Matrix>::edgeIter() {
    this->cache()->_generator.iter();
}

template <class Matrix>
std::optional<gsp::Edge> gsp::Graph<Matrix>::edgeNext() {
    return this->cache()->_generator.next();
}

template <class Matrix>
std::vector<gsp::Edge> gsp::Graph<Matrix>::edges() {
    return this->cache()->_generator.toVector();
}















template <class Matrix>
gsp::MatrixBox<Matrix>::MatrixBox(const Matrix* matrix) : _weights(matrix) {
}

template <class Matrix>
gsp::MatrixBox<Matrix>::~MatrixBox() {
    reset();
}


template <class Matrix>
void gsp::MatrixBox<Matrix>::reset() {
    this->_weights = nullptr;
    gsp::matrix::free(this->_laplacian);
    gsp::matrix::free(this->_normalized_laplacian);
    gsp::matrix::free(this->_normalized_weights);
}

template <class Matrix>
void gsp::MatrixBox<Matrix>::setWeights(Matrix* matrix) {
    reset();
    this->_weights = matrix;
}

template <class Matrix>
const Matrix& gsp::MatrixBox<Matrix>::weights() const{
    return *(this->_weights);
}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::weights() const {
    return this->_weights;
}

template <class Matrix>
gsp::CacheBox<Matrix>* gsp::Graph<Matrix>::cache() {
    if (not _cache) {
        _cache = new gsp::CacheBox<Matrix>(this);
    }
    return _cache;
}


template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::laplacian() {
    if (isCalculated(this->_laplacian)) {
        return this->_laplacian;
    }
    // degree[i] = sum_j W(i,j)
    densevector& cached_degrees = this->degrees();

    // L = D - W (dense, no extra temporaries)
    _laplacian = -*_weights;
    _laplacian.diagonal().array() += cached_degrees.array();

    return _laplacian;
}

template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::laplacian() {
    if (isCalculated(this->_laplacian)) {
        return this->_laplacian; // already computed
    }

    const auto n = static_cast<int>(_weights->rows());

    // degree[i] = sum_j W(i,j)
    densevector& cached_degrees = this->degrees();

    // Build L = D - W using triplets
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(static_cast<size_t>(_weights->nonZeros()) + n);

    // Off-diagonal: -W(i,j)
    for (int k = 0; k < _weights->outerSize(); ++k) {
        for (typename gsp::sparsematrix::InnerIterator it(*_weights, k); it; ++it) {
            if (it.row() != it.col()) {
                triplets.emplace_back(it.row(), it.col(), -it.value());
            }
        }
    }
    // Diagonal: degree[i] - W(i,i)  (keeps self-loop logic correct)
    for (int i = 0; i < n; ++i) {
        triplets.emplace_back(i, i, cached_degrees[i] - _weights->coeff(i, i));
    }

    _laplacian.resize(n, n);
    _laplacian.setFromTriplets(triplets.begin(), triplets.end());
    _laplacian.makeCompressed();

    return _laplacian;
}


template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::normalizedWeight() {
    if (isCalculated(this->_normalized_weights)) {
        return this->_normalized_weights; // already computed
    }
    densevector& cached_degrees = this->degrees();


    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });


    this->_normalized_weights = _weights->array().rowwise() * d_inv_sqrt.transpose().array(); // right scaling (columns)
    this->_normalized_weights = this->_normalized_weights.array().colwise() * d_inv_sqrt.array();  // left  scaling (rows)

    return _normalized_weights;
}


template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::normalizedWeight() {
    if (isCalculated(this->_normalized_weights)) {
        return this->_normalized_weights; // already computed
    }

    densevector& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    this->_normalized_weights = *_weights;

    using SparseT = std::remove_reference_t<decltype(_normalized_weights)>;

    for (int k = 0; k < this->_normalized_weights.outerSize(); ++k) {
        for (SparseT::InnerIterator it(_normalized_weights, k); it; ++it) {
            it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
        }
    }

    return _normalized_weights;
}




template <>
gsp::densematrix& gsp::MatrixBox<gsp::densematrix>::normalizedLaplacian() {
    if (isCalculated(this->_normalized_laplacian)) {
        return this->_normalized_laplacian;
    }

    densevector& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    const auto& cached_laplacian = this->laplacian();

    this->_normalized_laplacian = cached_laplacian.array().rowwise() * d_inv_sqrt.transpose().array(); // right scaling (columns)
    this->_normalized_laplacian = this->_normalized_laplacian.array().colwise() * d_inv_sqrt.array();  // left  scaling (rows)



    return _laplacian;
}

template <>
gsp::sparsematrix& gsp::MatrixBox<gsp::sparsematrix>::normalizedLaplacian() {
    if (isCalculated(this->_normalized_laplacian)) {
        return this->_normalized_laplacian; // already computed
    }

    const auto n = static_cast<int>(_weights->rows());

    densevector& cached_degrees = this->degrees();

    Eigen::VectorXd d_inv_sqrt = cached_degrees.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    auto& cached_laplacian = this->laplacian();

    this->_normalized_laplacian.swap(cached_laplacian);

    using SparseT = std::remove_reference_t<decltype(_normalized_laplacian)>;

    for (int k = 0; k < this->_normalized_laplacian.outerSize(); ++k) {
        for (SparseT::InnerIterator it(_normalized_laplacian, k); it; ++it) {
            it.valueRef() *= d_inv_sqrt[it.row()] * d_inv_sqrt[it.col()];
        }
    }

    return _normalized_laplacian;
}

template <class Matrix>
const typename gsp::MatrixBox<Matrix>::densevector& gsp::Graph<Matrix>::degrees() {
    return this->cache()->_matrix.degrees();
}



template <class Matrix>
const Matrix& gsp::Graph<Matrix>::laplacian() {
    return this->cache()->_matrix.laplacian();

}

template <class Matrix>
const Matrix& gsp::Graph<Matrix>::normalizedLaplacian() {
    return this->cache()->_matrix.normalizedLaplacian();

}


template <class Matrix>
typename gsp::MatrixBox<Matrix>::densevector& gsp::MatrixBox<Matrix>::degrees() {
    if (isCalculated(this->_degrees)) {
        return _degrees;
    }
    _degrees = *_weights * densevector::Ones(_weights->cols());
    return _degrees;
}



template <class Matrix>
gsp::CacheBox<Matrix>::CacheBox(gsp::Graph<Matrix>* graph) : _matrix(&graph->weights()), _generator(graph) {

}


template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const Matrix& m) {
    return m.rows() == _weights->rows();
}
template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const typename gsp::MatrixBox<Matrix>::densevector& m) {
    return m.rows() == _weights->rows();
}



template class gsp::Graph<gsp::densematrix>;  /// DenseMatrix
template class gsp::Graph<gsp::sparsematrix>; /// SparseMatrix

template class gsp::MatrixBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::MatrixBox<gsp::sparsematrix>; /// SparseMatrix

template class gsp::CacheBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::CacheBox<gsp::sparsematrix>; /// SparseMatrix


