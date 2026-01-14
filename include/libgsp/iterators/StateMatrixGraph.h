//
// Created by mohammad on 1/10/26.
//

#ifndef LIBGSP_STATEMATRIXGRAPH_H
#define LIBGSP_STATEMATRIXGRAPH_H
#pragma once

//#include "libgsp/iterators/EdgeGenerator2.h"
#include "libgsp/Graph.h"

namespace gsp {
    template <class Matrix> class StateMatrixGraph;
}

template <class Matrix>
class gsp::StateMatrixGraph : public gsp::BaseStateEdgeGenerator {
public:
    StateMatrixGraph(Matrix* weight, bool is_directed, double thresh);
    StateMatrixGraph(const StateMatrixGraph& other) = delete;
    StateMatrixGraph(const StateMatrixGraph* other);
    StateMatrixGraph& operator=(const StateMatrixGraph&) = delete;
    ~StateMatrixGraph();

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
