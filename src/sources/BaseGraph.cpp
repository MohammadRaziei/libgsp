//
// Created by Mohammad on 8/17/2025.
//

#include "libgsp/BaseGraph.h"


gsp::Edge::Edge(uint32_t source, uint32_t target, double weight,
        std::shared_ptr<gsp::BaseStateEdgeGenerator> state) :
    source_(source), target_(target), weight_(weight), state_(state) {}

void gsp::Edge::setWeight(double weight) {
    if (state_) state_->setWeight(weight);
    weight_ = weight;
}

gsp::Edge& gsp::Edge::operator=(double weight) { setWeight(weight); return *this; }
gsp::Edge& gsp::Edge::operator+=(double weight) { setWeight(weight_ + weight); return *this; }
gsp::Edge& gsp::Edge::operator-=(double weight) { setWeight(weight_ - weight); return *this; }
gsp::Edge& gsp::Edge::operator*=(double weight) { setWeight(weight_ * weight); return *this; }
gsp::Edge& gsp::Edge::operator/=(double weight) { setWeight(weight_ / weight); return *this; }

gsp::Edge gsp::Edge::detach() const {
    return gsp::Edge(source_, target_, weight_);
}

bool gsp::Edge::isDetached() const {
    return state_ == nullptr;
}

gsp::BaseGraph::BaseGraph(uint32_t num_nodes) : VertexGraph(num_nodes) {}
gsp::BaseGraph::~BaseGraph() {}
gsp::BaseGraph::BaseGraph(gsp::BaseGraph&& other) noexcept : VertexGraph(std::move(other)), is_directed_(other.is_directed_) {}
gsp::BaseGraph::BaseGraph(const gsp::BaseGraph *other) noexcept :
    gsp::VertexGraph(dynamic_cast<const gsp::VertexGraph*>(other)), is_directed_(other->is_directed_) {}
gsp::BaseGraph::BaseGraph(const gsp::VertexGraph *other) noexcept :
   gsp::VertexGraph(other) {}
gsp::BaseGraph::BaseGraph(gsp::VertexGraph &&other) noexcept :
        gsp::VertexGraph(std::move(other)) {}

gsp::BaseGraph& gsp::BaseGraph::operator=(gsp::BaseGraph&& other) noexcept {
    if (this != &other) {
        VertexGraph::operator=(std::move(other));
    }
    return *this;
}

std::vector<gsp::Edge> gsp::BaseGraph::edges(double thresh) const {
    // Create a temporary generator to get all edges
    auto gen = this->iterEdges(thresh);
    gen.reset(); // Reset to beginning
    std::vector<gsp::Edge> edges;
    while (auto edge = gen.next()) {
        edges.push_back(*edge);
    }
    return edges;
}


bool gsp::BaseGraph::isDirected() const {
    return this->is_directed_;
}

void gsp::BaseGraph::setIsDirectedUnsafe(bool is_directed) {
    this->is_directed_ = is_directed;
}

