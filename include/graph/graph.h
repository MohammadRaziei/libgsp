//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <ap.h>
#include <stdint.h>

#define GSP_IS_DIRECTED_DEFAULT false


namespace gsp {
    class VertexGraph;
    template <class matrix> class MatrixGraph;
    class SparseGraph;
    class DenseGraph;

    using Graph = SparseGraph;
}


class gsp::VertexGraph {
   public:
    VertexGraph(const uint32_t);
    VertexGraph& setCoords(const alglib::real_2d_array&);

   public:
    const uint32_t num_nodes;
    std::vector<std::string> names;
    alglib::real_2d_array coords; /// num_nodes x 2
};

template <class Matrix>
class gsp::MatrixGraph : public gsp::VertexGraph {
   public:
    MatrixGraph(const uint32_t num_nodes,
                const bool is_directed = GSP_IS_DIRECTED_DEFAULT):
          VertexGraph(num_nodes), is_directed(is_directed) {
    }
    MatrixGraph& setWeights(const Matrix& matrix){
        weights = matrix;
        return *this;
    }
   public:
    Matrix weights;
    bool is_directed;
};



class gsp::SparseGraph : public gsp::MatrixGraph<alglib::sparsematrix>{
   public:
    SparseGraph(const uint32_t num_nodes, const bool is_directed = GSP_IS_DIRECTED_DEFAULT);
};

class gsp::DenseGraph : public gsp::MatrixGraph<alglib::real_2d_array>{
   public:
    DenseGraph(const uint32_t);
};

#endif  // LIBGSP_GRAPH_H
