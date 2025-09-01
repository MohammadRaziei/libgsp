//
// Created by Mohammad on 8/17/2025.
//

#ifndef LIBGSP_BASEGRAPH_H
#define LIBGSP_BASEGRAPH_H
#pragma once


#include <optional>

#include "libgsp/graph/vertexgraph.h"

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
    virtual ~BaseGraph();
    void operator=(const gsp::BaseGraph& other) = delete;

    virtual void setEdges(const std::vector<gsp::Edge>& edges, bool is_directed = GSP_IS_DIRECTED_DEFAULT) = 0;
    virtual void edgeIter(double thresh = 0.) = 0;
    virtual std::optional<gsp::Edge> edgeNext() = 0;
    virtual std::vector<gsp::Edge> edges() const = 0;
};

#endif  // LIBGSP_BASEGRAPH_H
