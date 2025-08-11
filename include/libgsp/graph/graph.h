//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <cstdint>

#include <Eigen/Eigen>
#include "libgsp/utils/types.h"


#define GSP_IS_DIRECTED_DEFAULT false




namespace gsp {
    using densematrix = Eigen::MatrixXd;
    using sparsematrix = Eigen::SparseMatrix<double, Eigen::RowMajor>;


    struct Coord;
    struct Edge;
    enum class ShiftType;

    template<class Matrix> class MatrixBox;
    template<class Matrix> class CacheBox;

    class VertexGraph;
    template <class Matrix> class Graph;
    using SparseGraph = Graph<sparsematrix>;
    using DenseGraph = Graph<densematrix>;

}

struct gsp::Coord {
    Coord(double x, double y, double z = 0) : x(x), y(y), z(z) {}
    double x, y, z;
};


class gsp::VertexGraph {
   public:
    explicit VertexGraph(uint32_t);
    VertexGraph(const gsp::VertexGraph& other) = delete;
    void operator=(const gsp::VertexGraph& other) = delete;

    virtual ~VertexGraph();
    virtual void setCoords(const Eigen::MatrixXd& coords);
    virtual void setCoords(const std::vector<gsp::Coord>&);
    virtual void setNames(const std::vector<std::string>&);


   public:
    const uint32_t num_nodes;
    std::vector<std::string> names;


    using CoordMat = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
    CoordMat coords; // size fixed elsewhere to (num_nodes x 3)
};


struct gsp::Edge {
    Edge(uint32_t source, uint32_t target, double weight=1.0) :
          source(source), target(target), weight(weight) {}
    uint32_t source, target;
    double weight;
};


enum class gsp::ShiftType {
    Weights,
    Laplacian,
    NormalizedWeights,
    NormalizedLaplacian
};

template <class Matrix>
class gsp::Graph : public gsp::VertexGraph {
   public:
    explicit Graph(uint32_t num_nodes, const bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    Graph(const gsp::Graph<Matrix>& other) = delete;
    void operator=(const gsp::Graph<Matrix>& other) = delete;
    Graph(const gsp::VertexGraph& other);
    virtual ~Graph();

    virtual void setWeights(const Matrix& matrix);
    virtual void setWeights(const std::vector<gsp::Edge>& edges);

    virtual void validateWeights(const Matrix&);


    virtual const Matrix& weights() const;
    virtual const Matrix& laplacian();
    virtual const Matrix& normalizedLaplacian();
    virtual const typename gsp::MatrixBox<Matrix>::densevector& degrees();

    void invalidateCache();
   protected:
    bool is_directed;
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

    explicit MatrixBox(Matrix* weights);
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
private:
    Matrix* _weights, _laplacian, _normalized_weights, _normalized_laplacian;
    densevector _degrees;
};



template <class Matrix>
class gsp::CacheBox {
public:
    CacheBox(Matrix* weights = nullptr);
    gsp::MatrixBox<Matrix> matrix;
};




#endif  // LIBGSP_GRAPH_H
