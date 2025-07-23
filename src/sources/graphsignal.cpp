//
// Created by Mohammad on 7/22/2025.
//

#include "graph/graphsignal.h"


gsp::GraphSignal::GraphSignal(const gsp::Graph& graph,
                              const alglib::real_1d_array& signal)
    : graph(graph), signal(signal) {

}
