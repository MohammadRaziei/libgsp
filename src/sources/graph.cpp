//
// Created by Mohammad on 7/20/2025.
//
#include <linalg.h>

#include "libgsp/graph/graph.h"
#include "libgsp/utils/matrix.h"


gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes(num_nodes) {}

void gsp::VertexGraph::setCoords(const alglib::real_2d_array& coords) {
    this->coords = coords;
}
void gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    this->names = names;
}
void gsp::VertexGraph::setCoords(
    const std::vector<std::pair<double, double>>& coords) {
    this->coords.setcontent(this->num_nodes, 2, (double*) coords.data());
}
gsp::VertexGraph::~VertexGraph() {}

template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes, bool is_directed):
      VertexGraph(num_nodes), is_directed(is_directed) {
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix, int32_t num_edges){
    if (num_edges >= 0) {
        this->num_edges = num_edges;
    }
    /// TODO: calc num_edges from matrix
    this->weights = matrix;
}


template <class Matrix>
void gsp::Graph<Matrix>::setWeights(
    const std::vector<gsp::Edge>& edges, int32_t num_edges) {
    gsp::matrix::allocate(this->weights, this->num_nodes, this->num_nodes);
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight == 0)
            continue;
        gsp::matrix::setElement(this->weights, it->source, it->target, it->weight);
        if (!is_directed) {
            gsp::matrix::setElement(this->weights, it->target, it->source, it->weight);
        }
    }
}



template <class Matrix>
void gsp::Graph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::Graph<Matrix>::~Graph() {}

template class gsp::Graph<gsp::densematrix>;  /// DenseMatrix
template class gsp::Graph<gsp::sparsematrix>; /// SparseMatrix

