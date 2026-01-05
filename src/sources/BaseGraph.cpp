//
// Created by Mohammad on 8/17/2025.
//

#include "libgsp/BaseGraph.h"

gsp::BaseGraph::BaseGraph(uint32_t num_nodes) : VertexGraph(num_nodes) {}
gsp::BaseGraph::~BaseGraph() {}
gsp::BaseGraph::BaseGraph(gsp::BaseGraph&& other) noexcept : VertexGraph(std::move(other)) {}
gsp::BaseGraph::BaseGraph(const gsp::BaseGraph *other) noexcept :
    gsp::VertexGraph(dynamic_cast<const gsp::VertexGraph*>(other)) {}
gsp::BaseGraph::BaseGraph(const gsp::VertexGraph *other) noexcept  :
   gsp::VertexGraph(other) {}

