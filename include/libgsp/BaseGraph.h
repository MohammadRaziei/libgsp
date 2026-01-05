//
// Created by Mohammad on 8/17/2025.
//

#ifndef LIBGSP_BASEGRAPH_H
#define LIBGSP_BASEGRAPH_H
#pragma once


#include <optional>
#include <memory>

#include "VertexGraph.h"
#include "libgsp/iterators/EdgeGenerator.h"

#define GSP_IS_DIRECTED_DEFAULT false



namespace gsp {
struct Edge;
class BaseGraph;
}

struct gsp::Edge {
    Edge(uint32_t source, uint32_t target, double weight=1.0) :
          source(source), target(target), weight(weight) {}
    uint32_t source, target;
    double weight;
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

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) = 0;
    virtual std::vector<gsp::Edge> edges() const = 0;

    // Pure virtual method for polymorphic copying
    virtual std::unique_ptr<BaseGraph> copy() const = 0;

};

#endif  // LIBGSP_BASEGRAPH_H
