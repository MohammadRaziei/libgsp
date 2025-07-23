//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once

#include <linalg.h>

#include "graph/graph.h"

namespace gsp{
class GraphSignal {
   public:
    GraphSignal(const gsp::Graph&, const alglib::real_1d_array&);

   public:
    gsp::Graph graph;
    alglib::real_1d_array signal;
};
}

#endif  // LIBGSP_GRAPHSIGNAL_H
