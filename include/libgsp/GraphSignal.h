//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once


#include "Graph.h"
#include "libgsp/Signal.h"
#include "libgsp/utils/Logging.h"

// #include "libgsp/utils/types.h"
#include <Eigen/Sparse>
#include <Eigen/Dense>


namespace gsp {



template <class Matrix, class T>
class GraphSignal {
   public:
    GraphSignal(const gsp::Graph<Matrix>& graph, const gsp::Signal<T>& signal)
        : graph_(&graph), signal_(signal), logger_(gsp::logging::getLogger("GraphSignal")) {
        if (graph_->num_nodes != signal_.size()) {
            const std::string msg = fmt::format("Signal size {} does not match graph size {}", signal_.size(), graph_->num_nodes);
            logger_->error(msg);
            throw std::length_error(msg);
        }
    }

    gsp::Graph<Matrix>& graph() const { return *graph_; }
    gsp::Signal<T>& signal() { return signal_; }


   private:
    const gsp::Graph<Matrix>* graph_;
    gsp::Signal<T> signal_;
    gsp::logging::Logger logger_;
};
} // namespace gsp



#endif  // LIBGSP_GRAPHSIGNAL_H
