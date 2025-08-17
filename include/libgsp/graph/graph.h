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
#include "libgsp/graph/edgegenerator.h"




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
    explicit Graph(uint32_t num_nodes);
    Graph(const gsp::Graph<Matrix>& other) = delete;
    void operator=(const gsp::Graph<Matrix>& other) = delete;
    Graph(const gsp::VertexGraph& other);
    virtual ~Graph();

    virtual void edgeIter() override;
    virtual std::optional<gsp::Edge> edgeNext() override;
    virtual std::vector<gsp::Edge> edges() override;

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) override;
    virtual void setWeights(const Matrix& matrix, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual void setWeights(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT);

    virtual void validateWeights(const Matrix&);


    bool isDirected() const;
    void setIsDirectedUnsafe(bool);

    virtual const Matrix& weights() const;
    virtual const Matrix& laplacian();
    virtual const Matrix& normalizedLaplacian();
    virtual const typename gsp::MatrixBox<Matrix>::densevector& degrees();

    void invalidateCache();
   protected:
    bool _is_directed;
    Matrix _weights;

    gsp::ShiftType shift_type = gsp::ShiftType::Laplacian;
   private:
    gsp::CacheBox<Matrix>* cache();
    gsp::CacheBox<Matrix>* _cache;
};

template<class Matrix>
class gsp::MatrixBox {
public:
    using densevector = Eigen::Matrix<gsp::types::elem_t<Matrix>, Eigen::Dynamic, 1, Eigen::ColMajor>;

    MatrixBox() = default;
    MatrixBox(const MatrixBox&) = delete;
    MatrixBox& operator=(const MatrixBox&) = delete;

    explicit MatrixBox(const Matrix* weights);
    ~MatrixBox();

    void reset();

    void setWeights(Matrix* weights);

    const Matrix& weights() const;
    Matrix& normalizedWeight();
    Matrix& laplacian();
    Matrix& normalizedLaplacian();
    densevector& degrees();
private:
    bool isCalculated(const Matrix&);
    bool isCalculated(const densevector&);

    const Matrix* _weights;
    Matrix _laplacian, _normalized_weights, _normalized_laplacian;
    densevector _degrees;
};

template <class Matrix>
class gsp::CacheBox {
public:
    CacheBox(gsp::Graph<Matrix>* graph);

    gsp::EdgeGenerator<Matrix> _generator;
    gsp::MatrixBox<Matrix> _matrix;
};






#endif  // LIBGSP_GRAPH_H
