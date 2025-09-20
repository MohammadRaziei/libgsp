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
    GraphSignal(gsp::Graph<Matrix>& graph, const gsp::Signal<T>& signal)
        : _graph(&graph), _signal(signal), _logger(gsp::logging::getLogger("GraphSignal")) {
        if (_graph->num_nodes != _signal.size()) {
            const std::string msg = fmt::format("Signal size {} does not match graph size {}", _signal.size(), _graph->num_nodes);
            _logger->error(msg);
            throw std::length_error(msg);
        }
    }

    gsp::Graph<Matrix>& graph() const { return *_graph; }
    gsp::Signal<T>& signal() { return _signal; }


   private:
    gsp::Graph<Matrix>* _graph;
    gsp::Signal<T> _signal;
    gsp::logging::Logger _logger;
};
} // namespace gsp



#endif  // LIBGSP_GRAPHSIGNAL_H
