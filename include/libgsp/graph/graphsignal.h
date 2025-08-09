//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once


#include "libgsp/graph/graph.h"
// #include "libgsp/utils/types.h"


namespace gsp {

template <class Matrix, class Signal //,
    // typename = std::enable_if_t<
        // gsp::types::is_matrix<Matrix>::value &&
        // gsp::types::is_vector<Signal>::value
    // >
>
class GraphSignal {
   public:
    GraphSignal(gsp::Graph<Matrix>* graph,
                const Signal& signal);

   public:
    gsp::Graph<Matrix>* graph;
    Signal signal;
};
}


#endif  // LIBGSP_GRAPHSIGNAL_H
