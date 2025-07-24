//
// Created by Mohammad on 7/20/2025.
//
#include <linalg.h>

#include "graph/graph.h"
#include "utils/matrix.h"


gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes) {}

gsp::VertexGraph& gsp::VertexGraph::setCoords(const alglib::real_2d_array& coords) {
    this->coords = coords;
    return *this;
}
gsp::VertexGraph& gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    this->names = names;
    return *this;
}
gsp::VertexGraph& gsp::VertexGraph::setCoords(
    const std::vector<std::pair<double, double>>& coords) {
    this->coords.setcontent(this->num_nodes, 2, (double*) coords.data());
    return *this;
}
gsp::VertexGraph::~VertexGraph() {}

template <class Matrix>
gsp::MatrixGraph<Matrix>::MatrixGraph(uint32_t num_nodes, const bool is_directed):
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


template <class Matrix>
gsp::MatrixGraph<Matrix>& gsp::MatrixGraph<Matrix>::setWeights(
    const std::vector<std::pair<int, int>>& edges,
    const bool auto_validate) {
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        gsp::matrix::allocate(this->weights, this->num_nodes, this->num_nodes);
        gsp::matrix::setElement(this->weights, it->first, it->second, 1.);
        if (!is_directed) {
            gsp::matrix::setElement(this->weights, it->second, it->first, 1.);
        }
    }
    return *this;
}



template <class Matrix>
void gsp::MatrixGraph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::MatrixGraph<Matrix>::~MatrixGraph() {}

template class gsp::MatrixGraph<alglib::real_2d_array>; /// DenseMatrix
template class gsp::MatrixGraph<alglib::sparsematrix>;  /// SparseMatrix

