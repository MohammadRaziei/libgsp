//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <cstdint>

#include "libgsp/utils/types.h"
#include "libgsp/graph/basegraph.h"




namespace gsp {
    enum class ShiftType;

    template<class Matrix> class MatrixBox;
    template<class Matrix> class CacheBox;

    class BaseGraph;
    template <class Matrix> class Graph;
    using SparseGraph = Graph<sparsematrix>;
    using DenseGraph = Graph<densematrix>;

}



enum class gsp::ShiftType {
    Weights,
    Laplacian,
    NormalizedWeights,
    NormalizedLaplacian
};




template <class Matrix>
class gsp::Graph : public gsp::BaseGraph {
   public:
    using densevector = typename Eigen::Matrix<gsp::types::elem_t<Matrix>, Eigen::Dynamic, 1, Eigen::ColMajor>;

   public:
    explicit Graph(uint32_t num_nodes);
    Graph(const gsp::Graph<Matrix>& other) = delete;
    void operator=(const gsp::Graph<Matrix>& other) = delete;
    Graph(const gsp::VertexGraph& other);
    virtual ~Graph();

    virtual void edgeIter(double thresh = 0.) override;
    virtual std::optional<gsp::Edge> edgeNext() override;
    virtual std::vector<gsp::Edge> edges() const override;

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) override;
    virtual void setWeights(const Matrix& matrix, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual void setWeights(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT);

    virtual void validateWeights(const Matrix&);


    bool isDirected() const;
    void setIsDirectedUnsafe(bool);

    virtual const Matrix& weights() const;
    virtual const Matrix& laplacian();
    virtual const Matrix& normalizedLaplacian();
    virtual const  densevector& degrees();

    void invalidateCache();
   protected:
    bool _is_directed;
    Matrix _weights;

    gsp::ShiftType shift_type = gsp::ShiftType::Laplacian;
   private:
    gsp::CacheBox<Matrix>* cache();
    gsp::CacheBox<Matrix>* _cache;
};






#endif  // LIBGSP_GRAPH_H
