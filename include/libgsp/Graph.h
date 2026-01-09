//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H
#pragma once

#include <vector>
#include <cstdint>
#include <memory>

#include "BaseGraph.h"
#include "libgsp/utils/Types.h"
#include "libgsp/utils/Logging.h"

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
    Graph(gsp::Graph<Matrix>&& other) noexcept;
    explicit Graph(const gsp::Graph<Matrix>* other) noexcept;
    explicit Graph(const gsp::VertexGraph* other) noexcept;
    virtual ~Graph();

    void operator=(const gsp::Graph<Matrix>& other) = delete;
    gsp::Graph<Matrix>& operator=(gsp::Graph<Matrix>&& other) noexcept;

    virtual std::vector<gsp::Edge> edges(double thresh = 0) const override;

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) override;
    virtual void setEdges(gsp::EdgeGenerator<Matrix>& generator, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual void setWeights(const Matrix& matrix, bool is_directed = GSP_IS_DIRECTED_DEFAULT);
    virtual void setWeights(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT);

    virtual void validateWeights(const Matrix&);


    bool isDirected() const;
    void setIsDirectedUnsafe(bool);

    virtual const Matrix& weights() const;
    virtual const Matrix& laplacian();
    virtual const Matrix& normalizedLaplacian();
    virtual const  densevector& degrees();

    // New method for edge iteration using EdgeGenerator
    gsp::EdgeGenerator<Matrix> iterEdges(double thresh = 0.0) const;
    // Clone method to create a deep copy of the graph (value-based, preserves concrete type)
    gsp::Graph<Matrix> clone() const;
    // Graph conversion methods

    template<class Target> gsp::Graph<Target> to(double thresh = 0.0) const {
        if constexpr (std::is_same_v<Matrix, Target>) {
            if (thresh == 0) {
                logger_->warn("Warning: Graph is already dense!");
                return clone();
            }
            Graph<Target> graph(dynamic_cast<const VertexGraph*>(this));
            graph.setEdges(this->edges(thresh), this->is_directed_);
        }
        Graph<Target> graph(dynamic_cast<const VertexGraph*>(this));
        if constexpr (std::is_same_v<Matrix, Target>) {
            auto gen = this->iterEdges(thresh);
            graph.setEdges(gen, this->is_directed_);
        } else {
            graph.setEdges(this->edges(thresh), this->is_directed_);
        }
        return graph;
    }
    SparseGraph toSparse(double thresh = 0.0) const;
    DenseGraph toDense(double thresh = 0.0) const;

    void invalidateCache();
   protected:
    bool is_directed_;
    Matrix weights_;

    gsp::logging::Logger logger_;

private:
    gsp::CacheBox<Matrix>* cache();
    std::string detType() const;

    gsp::CacheBox<Matrix>* cache_ = nullptr;
};


#include "libgsp/iterators/EdgeGenerator.h"


#endif  // LIBGSP_GRAPH_H
