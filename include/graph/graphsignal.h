//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once

#include <linalg.h>

#include "graph/graph.h"

namespace gsp {
template <class Matrix>
class GraphSignal;
}

template <class Matrix>
class gsp::GraphSignal {
   public:
    GraphSignal(const gsp::MatrixGraph<Matrix>& graph,
                const alglib::real_1d_array& signal);

   public:
    gsp::MatrixGraph<Matrix> graph;
    alglib::real_1d_array signal;
};


#endif  // LIBGSP_GRAPHSIGNAL_H
