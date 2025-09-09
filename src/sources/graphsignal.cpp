//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/graph/graphsignal.h"


template <class Matrix, class Signal>
gsp::GraphSignal<Matrix, Signal>::GraphSignal(gsp::Graph<Matrix>* graph,
                              const Signal& signal)
    : graph(graph), signal(signal) {
    if (graph->num_nodes != signal.size()) {
        throw std::length_error("");
    }
}




// for DenseMatrix
template class gsp::GraphSignal<gsp::densematrix,  Eigen::VectorXd>;
template class gsp::GraphSignal<gsp::densematrix,  Eigen::VectorXcd>;
template class gsp::GraphSignal<gsp::densematrix,  Eigen::Map<Eigen::VectorXd>>;
template class gsp::GraphSignal<gsp::densematrix,  Eigen::Map<Eigen::VectorXcd>>;
// for SparseMatrix
template class gsp::GraphSignal<gsp::sparsematrix, Eigen::VectorXd>;
template class gsp::GraphSignal<gsp::sparsematrix, Eigen::VectorXcd>;
template class gsp::GraphSignal<gsp::sparsematrix, Eigen::Map<Eigen::VectorXd>>;
template class gsp::GraphSignal<gsp::sparsematrix, Eigen::Map<Eigen::VectorXcd>>;