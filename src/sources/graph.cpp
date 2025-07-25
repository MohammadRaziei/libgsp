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
gsp::MatrixGraph<Matrix>::MatrixGraph(uint32_t num_nodes, bool is_directed):
      VertexGraph(num_nodes), is_directed(is_directed) {
}

template <class Matrix>
gsp::MatrixGraph<Matrix>& gsp::MatrixGraph<Matrix>::setWeights(const Matrix& matrix, bool auto_validate){
    if (auto_validate) {
        validateWeights(weights);
    }
    this->weights = matrix;
    return *this;
}


template <class Matrix>
gsp::MatrixGraph<Matrix>& gsp::MatrixGraph<Matrix>::setWeights(
    const std::vector<gsp::Edge>& edges,
    const bool auto_validate) {
    gsp::matrix::allocate(this->weights, this->num_nodes, this->num_nodes);
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight == 0)
            continue;
        gsp::matrix::setElement(this->weights, it->source, it->target, it->weight);
        if (!is_directed) {
            gsp::matrix::setElement(this->weights, it->target, it->source, it->weight);
        }
    }
    return *this;
}



template <class Matrix>
void gsp::MatrixGraph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::MatrixGraph<Matrix>::~MatrixGraph() {}

template class gsp::MatrixGraph<gsp::densematrix>;  /// DenseMatrix
template class gsp::MatrixGraph<gsp::sparsematrix>; /// SparseMatrix

