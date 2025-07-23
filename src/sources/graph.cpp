//
// Created by Mohammad on 7/20/2025.
//

#include "graph/graph.h"


gsp::VertexGraph::VertexGraph(const uint32_t num_nodes) : num_nodes(num_nodes) {}

gsp::VertexGraph& gsp::VertexGraph::setCoords(const alglib::real_2d_array& coords) {
    this->coords = coords;
    return *this;
}
gsp::VertexGraph& gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    this->names = names;
    return *this;
}

template <class Matrix>
gsp::MatrixGraph<Matrix>::MatrixGraph(const uint32_t num_nodes, const bool is_directed):
      VertexGraph(num_nodes), is_directed(is_directed) {
}

template <class Matrix>
gsp::MatrixGraph<Matrix>& gsp::MatrixGraph<Matrix>::setWeights(const Matrix& matrix, const bool auto_validate){
    if (auto_validate) {
        validateWeights(weights);
    }
    this->weights = matrix;
    return *this;
}


template class gsp::MatrixGraph<alglib::real_2d_array>; /// DenseMatrix
template class gsp::MatrixGraph<alglib::sparsematrix>;  /// SparseMatrix

