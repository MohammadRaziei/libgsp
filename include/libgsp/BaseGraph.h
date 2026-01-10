//
// Created by Mohammad on 8/17/2025.
//

#ifndef LIBGSP_BASEGRAPH_H
#define LIBGSP_BASEGRAPH_H
#pragma once


#include <optional>
#include <memory>

#include "VertexGraph.h"

#define GSP_IS_DIRECTED_DEFAULT true



namespace gsp {
    class EdgeGenerator;
    class ConstEdgeGenerator;
    class BaseStateEdgeGenerator;
    class Edge;
    class BaseGraph;
}

class gsp::Edge {
public:
    Edge(uint32_t source, uint32_t target, double weight = 1.0,
         std::shared_ptr<gsp::BaseStateEdgeGenerator> state = nullptr);
    Edge& operator=(const gsp::Edge& other) = delete;

    Edge& operator=(double weight);
    Edge& operator+=(double weight);
    Edge& operator-=(double weight);
    Edge& operator*=(double weight);
    Edge& operator/=(double weight);

    [[nodiscard]] double weight() const { return weight_; }
    [[nodiscard]] const uint32_t source() const { return source_; }
    [[nodiscard]] const uint32_t target() const { return target_; }

    void setWeight(double weight);

    gsp::Edge detach() const;
    bool isDetached() const;

private:
    const uint32_t source_, target_;
    double weight_;
    std::shared_ptr<gsp::BaseStateEdgeGenerator> state_;
};


class gsp::BaseGraph : public gsp::VertexGraph {
   public:
    BaseGraph(uint32_t num_nodes);
    BaseGraph(const gsp::BaseGraph& other) = delete;
    BaseGraph(gsp::BaseGraph&& other) noexcept;
    explicit BaseGraph(const gsp::BaseGraph* other) noexcept;
    explicit BaseGraph(const gsp::VertexGraph* other) noexcept;
    virtual ~BaseGraph();
    void operator=(const gsp::BaseGraph& other) = delete;
    BaseGraph& operator=(gsp::BaseGraph&& other) noexcept;

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) = 0;

    virtual gsp::ConstEdgeGenerator iterEdges(double thresh = 0.0) const = 0;
    virtual gsp::EdgeGenerator iterEdges(double thresh = 0.0) = 0;

    virtual std::vector<gsp::Edge> edges(double thresh = 0.0) const;

};

#include "libgsp/iterators/EdgeGenerator.h"


#endif  // LIBGSP_BASEGRAPH_H
