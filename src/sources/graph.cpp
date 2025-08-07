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
    const std::vector<gsp::Coord>& coords) {
    this->coords.setcontent(this->num_nodes, 3, (double*) coords.data());
}
gsp::VertexGraph::~VertexGraph() {}

template <class Matrix>
gsp::Graph<Matrix>::Graph(uint32_t num_nodes, bool is_directed):
      VertexGraph(num_nodes), is_directed(is_directed) {
}

template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const Matrix& matrix){
    this->weights = matrix;
}


template <class Matrix>
void gsp::Graph<Matrix>::setWeights(const std::vector<gsp::Edge>& edges) {
    gsp::matrix::free(this->weights);
    gsp::matrix::allocate(this->weights, this->num_nodes, this->num_nodes);
    this->num_edges = 0;
    for (auto it = edges.begin(); it < edges.end(); ++it) {
        if (it->weight == 0) continue;
        gsp::matrix::setElement(this->weights, it->source, it->target, it->weight);
        ++(this->num_edges);
        if (!is_directed) {
            double w = gsp::matrix::getElement(this->weights, it->target, it->source);
            if (w == 0) {
                gsp::matrix::setElement(this->weights, it->target, it->source, it->weight);
            } else if (w != it->weight) {
                gsp::matrix::free(this->weights);
                this->num_edges = -1;
                throw std::invalid_argument("Weights for undirected edges must be equal.");
            } else {
                --(this->num_edges); // Already counted
            }
        }
    }
}



template <class Matrix>
void gsp::Graph<Matrix>::validateWeights(const Matrix&) {}

template <class Matrix>
gsp::Graph<Matrix>::~Graph() { gsp::matrix::free(this->weights);}

template class gsp::Graph<gsp::densematrix>;  /// DenseMatrix
template class gsp::Graph<gsp::sparsematrix>; /// SparseMatrix

