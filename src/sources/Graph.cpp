//
// Created by Mohammad on 7/20/2025.
//
#include  <Eigen/Eigen>

#include "libgsp/iterators/EdgeGenerator.h"
#include "libgsp/Graph.h"
#include "libgsp/utils/Matrix.h"
#include <ciso646>
#include <type_traits>
#include <iostream>

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

    const Matrix* _weights;
    Matrix _laplacian, _normalized_weights, _normalized_laplacian;
    typename gsp::Graph<Matrix>::densevector _degrees;
};

template <class Matrix>
class gsp::CacheBox {
   public:
    CacheBox(gsp::Graph<Matrix>* graph);

    gsp::MatrixBox<Matrix> _matrix;
};


template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes):
      BaseGraph(num_nodes), _cache(nullptr), _is_directed(GSP_IS_DIRECTED_DEFAULT) {
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::Graph<Matrix> *other) noexcept :
    gsp::BaseGraph(dynamic_cast<const BaseGraph*>(other)), _cache(nullptr),
    _is_directed(other->_is_directed), _weights(other->_weights), shift_type(other->shift_type) {
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(gsp::Graph<Matrix> &&other) noexcept
    : BaseGraph(std::move(other)), _weights(std::move(other._weights)),
      _is_directed(other._is_directed), shift_type(other.shift_type), _cache(other._cache) {
    // Reset the moved-from object's cache pointer
    other._cache = nullptr;
}


template<class Matrix>
gsp::Graph<Matrix>::Graph(const gsp::VertexGraph *other) noexcept: gsp::BaseGraph(other) {
}


template<class Matrix>
gsp::Graph<Matrix>& gsp::Graph<Matrix>::operator=(gsp::Graph<Matrix> &&other) noexcept {
    if (this == &other) return *this;

    // Call base class move assignment operator
    BaseGraph::operator=(std::move(other));

    // Move the matrix
    _weights = std::move(other._weights);

    // Move other members
    _is_directed = other._is_directed;
    shift_type = other.shift_type;

    // Move the cache
    delete _cache;  // Clean up existing cache
    _cache = other._cache;
    other._cache = nullptr;  // Reset moved-from object's cache pointer

    return *this;
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix, bool is_directed){
    invalidateCache();
    this->_weights = matrix;
    this->_is_directed = is_directed;
}


template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const std::vector<gsp::Edge>& edges, bool is_directed) {
    setEdges(edges, is_directed);
}

template <class Matrix>
void gsp::Graph<Matrix>::setEdges(const std::vector<gsp::Edge>& edges, bool is_directed) {
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
void gsp::Graph<Matrix>::setIsDirectedUnsafe(bool is_directed) {
    this->_is_directed = is_directed;
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
std::vector<gsp::Edge> gsp::Graph<Matrix>::edges() const {
    // Create a temporary generator to get all edges
    auto gen = this->iterEdges();
    std::vector<gsp::Edge> edges;
    gen.reset(); // Reset to beginning
    while (auto edge = gen.next()) {
        edges.push_back(*edge);
    }
    return edges;
}

template <class Matrix>
gsp::EdgeGenerator<Matrix> gsp::Graph<Matrix>::iterEdges(gsp::types::elem_t<Matrix> thresh) const {
    return gsp::EdgeGenerator<Matrix>(&this->_weights, this->num_nodes, this->_is_directed, thresh);
}

template <class Matrix>
std::unique_ptr<gsp::Graph<Matrix>> gsp::Graph<Matrix>::clone() const {
    auto cloned_graph = std::make_unique<gsp::Graph<Matrix>>(this->num_nodes);

    // Copy all properties from the current graph
    cloned_graph->_weights = this->_weights;  // This performs a deep copy for Eigen matrices
    cloned_graph->_is_directed = this->_is_directed;
    cloned_graph->shift_type = this->shift_type;

    // Copy VertexGraph properties (coordinates and names) using public interface
    // Copy names
    if (!this->names.empty()) {
        cloned_graph->setNames(this->names);
    }

    // Copy coordinates
    for (uint32_t i = 0; i < this->num_nodes; ++i) {
        auto coord = this->coord(i);
        cloned_graph->setCoord(i, coord);
    }

    // Note: Internal caches are not copied and will be recreated when needed
    // The cloned graph starts with a clean cache state (nullptr)
    cloned_graph->invalidateCache();

    return cloned_graph;
}

template <class Matrix>
std::unique_ptr<gsp::BaseGraph> gsp::Graph<Matrix>::copy() const {
    // Delegate to the clone method to maintain deep copy semantics
    return std::unique_ptr<gsp::BaseGraph>(this->clone().release());
}

// Implementation of toSparse method
template <class Matrix>
std::unique_ptr<gsp::SparseGraph> gsp::Graph<Matrix>::toSparse() const {
    // Check if this is already a sparse graph
    if constexpr (std::is_same_v<Matrix, gsp::sparsematrix>) {
        // Emit warning and return a clone
        std::cerr << "Warning: Graph is already sparse, returning clone" << std::endl;
        auto cloned = this->clone();
        auto result = std::make_unique<gsp::SparseGraph>(cloned->num_nodes);
        result->_weights = cloned->_weights;
        result->_is_directed = cloned->_is_directed;
        result->shift_type = cloned->shift_type;
        if (!cloned->names.empty()) {
            result->names = cloned->names;
        }
        for (uint32_t i = 0; i < cloned->num_nodes; ++i) {
            auto coord = cloned->coord(i);
            result->setCoord(i, coord);
        }
        result->invalidateCache();
        return result;
    } else {
        // Convert from dense to sparse
        auto sparse_graph = std::make_unique<gsp::SparseGraph>(this->num_nodes);

        // Use iterEdges to efficiently convert
        auto generator = this->iterEdges(0.0); // Include all edges with weight > 0

        std::vector<gsp::Edge> edges;
        while (auto edge = generator.next()) {
            edges.push_back(*edge);
        }

        sparse_graph->setEdges(edges, this->_is_directed);
        sparse_graph->invalidateCache();
        return sparse_graph;
    }
}

// Implementation of toDense method
template <class Matrix>
std::unique_ptr<gsp::DenseGraph> gsp::Graph<Matrix>::toDense() const {
    // Check if this is already a dense graph
    if constexpr (std::is_same_v<Matrix, gsp::densematrix>) {
        // Emit warning and return a clone
        std::cerr << "Warning: Graph is already dense, returning clone" << std::endl;
        auto cloned = this->clone();
        auto result = std::make_unique<gsp::DenseGraph>(cloned->num_nodes);
        result->_weights = cloned->_weights;
        result->_is_directed = cloned->_is_directed;
        result->shift_type = cloned->shift_type;
        if (!cloned->names.empty()) {
            result->names = cloned->names;
        }
        for (uint32_t i = 0; i < cloned->num_nodes; ++i) {
            auto coord = cloned->coord(i);
            result->setCoord(i, coord);
        }
        result->invalidateCache();
        return result;
    } else {
        // Convert from sparse to dense
        auto dense_graph = std::make_unique<gsp::DenseGraph>(this->num_nodes);

        // Use iterEdges to efficiently convert
        auto generator = this->iterEdges(0.0); // Include all edges with weight > 0

        std::vector<gsp::Edge> edges;
        while (auto edge = generator.next()) {
            edges.push_back(*edge);
        }

        dense_graph->setEdges(edges, this->_is_directed);
        dense_graph->invalidateCache();
        return dense_graph;
    }
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
    auto& cached_degrees = this->degrees();

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
    auto& cached_degrees = this->degrees();

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
    auto& cached_degrees = this->degrees();


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

    auto& cached_degrees = this->degrees();

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

    auto& cached_degrees = this->degrees();

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

    auto& cached_degrees = this->degrees();

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
const typename gsp::Graph<Matrix>::densevector& gsp::Graph<Matrix>::degrees() {
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
typename gsp::Graph<Matrix>::densevector& gsp::MatrixBox<Matrix>::degrees() {
    if (isCalculated(this->_degrees)) {
        return _degrees;
    }
    _degrees = *_weights * gsp::Graph<Matrix>::densevector::Ones(_weights->cols());
    return _degrees;
}



template <class Matrix>
gsp::CacheBox<Matrix>::CacheBox(gsp::Graph<Matrix>* graph) : _matrix(&graph->weights()) {

}


template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const Matrix& m) {
    return m.rows() == _weights->rows();
}
template <class Matrix>
bool gsp::MatrixBox<Matrix>::isCalculated(const typename gsp::Graph<Matrix>::densevector& m) {
    return m.rows() == _weights->rows();
}



template class gsp::Graph<gsp::densematrix>;  /// DenseMatrix
template class gsp::Graph<gsp::sparsematrix>; /// SparseMatrix

template class gsp::MatrixBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::MatrixBox<gsp::sparsematrix>; /// SparseMatrix

template class gsp::CacheBox<gsp::densematrix>;  /// DenseMatrix
template class gsp::CacheBox<gsp::sparsematrix>; /// SparseMatrix




