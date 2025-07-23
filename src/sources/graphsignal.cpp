//
// Created by Mohammad on 7/22/2025.
//

#include "graph/graphsignal.h"


template <class Matrix>
gsp::GraphSignal<Matrix>::GraphSignal(const gsp::MatrixGraph<Matrix>& graph,
                              const alglib::real_1d_array& signal)
    : graph(graph), signal(signal) {}





template class gsp::GraphSignal<alglib::real_2d_array>;
template class gsp::GraphSignal<alglib::sparsematrix>;
