//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/graph/graphsignal.h"


template <class Matrix, class Signal>
gsp::GraphSignal<Matrix, Signal>::GraphSignal(const gsp::Graph<Matrix>& graph,
                              const Signal& signal)
    : graph(graph), signal(signal) {
    if (graph.num_nodes != signal.length()) {
        throw std::length_error("");
    }
}





template class gsp::GraphSignal<alglib::real_2d_array, alglib::real_1d_array>;    /// for DenseMatrix
template class gsp::GraphSignal<alglib::real_2d_array, alglib::complex_1d_array>; /// for DenseMatrix
template class gsp::GraphSignal<alglib::sparsematrix,  alglib::real_1d_array>;    /// for SparseMatrix
template class gsp::GraphSignal<alglib::sparsematrix,  alglib::complex_1d_array>; /// for SparseMatrix
