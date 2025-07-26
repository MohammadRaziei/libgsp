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
    struct Edge;
    template <class matrix> class Graph;
    using SparseGraph = Graph<sparsematrix>;
    using DenseGraph = Graph<densematrix>;

}


class gsp::VertexGraph {
   public:
    explicit VertexGraph(uint32_t);
    virtual ~VertexGraph();
    virtual void setCoords(const alglib::real_2d_array&);
    virtual void setCoords(const std::vector<std::pair<double,double>>&);
    virtual void setNames(const std::vector<std::string>&);


   public:
    const uint32_t num_nodes;
    std::vector<std::string> names;
    alglib::real_2d_array coords; /// num_nodes x 2
};


struct gsp::Edge {
    Edge(uint32_t source, uint32_t target, double weight=1.0) :
          source(source), target(target), weight(weight) {}
    uint32_t source, target;
    double weight;
};

template <class Matrix>
class gsp::Graph : public gsp::VertexGraph {
   public:
    explicit Graph(uint32_t num_nodes, const bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual ~Graph() override;
    virtual void setWeights(const Matrix& matrix, int32_t num_edges = -1);
    virtual void setWeights(const std::vector<gsp::Edge>& edges, int32_t num_edges = -1);

    virtual void validateWeights(const Matrix&);


   public:
    Matrix weights;
   protected:
    int32_t num_edges = 0;
    bool is_directed;
};


#endif  // LIBGSP_GRAPH_H
