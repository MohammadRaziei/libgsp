//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <stdint.h>
#include <utility>

#include <ap.h>


#define GSP_IS_DIRECTED_DEFAULT false


namespace gsp {
    class VertexGraph;
    template <class matrix> class MatrixGraph;
    using SparseGraph = MatrixGraph<alglib::sparsematrix>;
    using DenseGraph = MatrixGraph<alglib::real_2d_array>;
}


class gsp::VertexGraph {
   public:
    VertexGraph(const uint32_t);
    VertexGraph& setCoords(const alglib::real_2d_array&);
    VertexGraph& setCoords(const std::vector<std::pair<double,double>>&);
    VertexGraph& setNames(const std::vector<std::string>&);


   public:
    const uint32_t num_nodes;
    std::vector<std::string> names;
    alglib::real_2d_array coords; /// num_nodes x 2
};

template <class Matrix>
class gsp::MatrixGraph : public gsp::VertexGraph {
   public:
    MatrixGraph(const uint32_t num_nodes, const bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    MatrixGraph<Matrix>& setWeights(const Matrix& matrix, const bool auto_validate=false);
    MatrixGraph<Matrix>& setWeights(const std::vector<std::pair<int, int>>& edges, const bool auto_validate=false);

    void validateWeights(const Matrix&);
   public:
    Matrix weights;
   protected:
    bool is_directed;
};

#endif  // LIBGSP_GRAPH_H
