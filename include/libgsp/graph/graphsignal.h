//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once

#include <linalg.h>

#include "libgsp/graph/graph.h"

namespace gsp {
template <class Matrix, class Signal> class GraphSignal;
}

template <class Matrix, class Signal>
class gsp::GraphSignal {
   public:
    GraphSignal(const gsp::Graph<Matrix>& graph,
                const Signal& signal);

   public:
    gsp::Graph<Matrix> graph;
    Signal signal;
};


#endif  // LIBGSP_GRAPHSIGNAL_H
