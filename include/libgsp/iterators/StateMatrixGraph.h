//
// Created by mohammad on 1/10/26.
//

#ifndef LIBGSP_STATEGRAPH_H
#define LIBGSP_STATEGRAPH_H
#pragma once

//#include "libgsp/iterators/EdgeGenerator2.h"
#include "libgsp/Graph.h"

namespace gsp {
    template <class Matrix> class StateGraph;
}

template <class Matrix>
class gsp::StateGraph : public gsp::BaseStateEdgeGenerator {
public:
    StateGraph(Matrix* weight, uint32_t num_nodes, bool is_directed, double thresh);
    StateGraph(const StateGraph& other) = delete;
    StateGraph(const StateGraph* other);
    StateGraph& operator=(const StateGraph&) = delete;
    ~StateGraph();

    virtual void reset() override;
    virtual std::optional<gsp::Edge> next() override;
    virtual std::shared_ptr<gsp::BaseStateEdgeGenerator> clone() const override;
    virtual void setWeight(double weight);


private:
        // common state (used by specializations)
        Matrix* weights_ = nullptr;
        uint32_t num_nodes_;
        double thresh_;
        bool is_directed_;

        class State;
        std::unique_ptr<State> state_;  // Using PIMPL pattern for state
    };



#endif //LIBGSP_STATEDENSEGRAPH_H
