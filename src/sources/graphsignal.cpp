//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/graph/graphsignal.h"


template <class Matrix, class Signal>
gsp::GraphSignal<Matrix, Signal>::GraphSignal(gsp::Graph<Matrix>* graph,
                              const Signal& signal)
    : graph(graph), signal(signal) {
    if (graph->num_nodes != signal.length()) {
        throw std::length_error("");
    }
}




// for DenseMatrix
template class gsp::GraphSignal<gsp::densematrix,  alglib::real_1d_array>;    
template class gsp::GraphSignal<gsp::densematrix,  alglib::complex_1d_array>; 
// for SparseMatrix
template class gsp::GraphSignal<gsp::sparsematrix, alglib::real_1d_array>;    
template class gsp::GraphSignal<gsp::sparsematrix, alglib::complex_1d_array>; 
