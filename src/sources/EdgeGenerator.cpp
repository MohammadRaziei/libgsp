//
// Created by Mohammad on 1/3/2026.
//

#include "libgsp/iterators/EdgeGenerator.h"

gsp::EdgeGenerator::EdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator> state) : state_(state) {}
gsp::ConstEdgeGenerator::ConstEdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator> state) : state_(state) {}

gsp::EdgeGenerator::~EdgeGenerator() {}
gsp::ConstEdgeGenerator::~ConstEdgeGenerator() {}

void gsp::EdgeGenerator::reset() { state_->reset(); }
void gsp::ConstEdgeGenerator::reset() { state_->reset(); }

std::optional<gsp::Edge> gsp::EdgeGenerator::next() { return state_->next(); }
const std::optional<const gsp::Edge> gsp::ConstEdgeGenerator::next() const { return state_->next(); }
//const std::optional<const gsp::Edge> gsp::EdgeGenerator::next() const { return state_->next(); }

gsp::BaseStateEdgeGenerator::BaseStateEdgeGenerator() {}
gsp::BaseStateEdgeGenerator::~BaseStateEdgeGenerator() {}
