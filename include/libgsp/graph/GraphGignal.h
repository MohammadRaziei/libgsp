//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once


#include "libgsp/graph/Graph.h"
#include "libgsp/utils/Logging.h"

// #include "libgsp/utils/types.h"


namespace gsp {

template <class Matrix, class Signal>
class GraphSignal {
   public:
    GraphSignal(gsp::Graph<Matrix>& graph,
                const Signal& signal);

   public:
    gsp::Graph<Matrix>* _graph;
    Signal _signal;
    gsp::logging::Logger _logger;
};
}


#endif  // LIBGSP_GRAPHSIGNAL_H
