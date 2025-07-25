//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <cstdint>
#include <utility>

#include <ap.h>


#define GSP_IS_DIRECTED_DEFAULT false




namespace gsp {
    using densematrix = alglib::real_2d_array;
    using sparsematrix = alglib::sparsematrix;


    class VertexGraph;
    template <class matrix> class MatrixGraph;
    using SparseGraph = MatrixGraph<sparsematrix>;
    using DenseGraph = MatrixGraph<densematrix>;

}


class gsp::VertexGraph {
   public:
    explicit VertexGraph(uint32_t);
    virtual ~VertexGraph();
    virtual VertexGraph& setCoords(const alglib::real_2d_array&);
    virtual VertexGraph& setCoords(const std::vector<std::pair<double,double>>&);
    virtual VertexGraph& setNames(const std::vector<std::string>&);


   public:
    const uint32_t num_nodes;
    std::vector<std::string> names;
    alglib::real_2d_array coords; /// num_nodes x 2
};

template <class Matrix>
class gsp::MatrixGraph : public gsp::VertexGraph {
   public:
    explicit MatrixGraph(uint32_t num_nodes, const bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual ~MatrixGraph() override;
    virtual MatrixGraph<Matrix>& setWeights(const Matrix& matrix, bool auto_validate=false);
    virtual MatrixGraph<Matrix>& setWeights(const std::vector<std::pair<int, int>>& edges, bool auto_validate=false);

    virtual void validateWeights(const Matrix&);


   public:
    Matrix weights;
   protected:
    bool is_directed;
};


#endif  // LIBGSP_GRAPH_H
