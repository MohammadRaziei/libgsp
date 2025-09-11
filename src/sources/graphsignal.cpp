//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/graph/graphsignal.h"

template <class Matrix, class Signal>
gsp::GraphSignal<Matrix, Signal>::GraphSignal(gsp::Graph<Matrix>& graph,
                              const Signal& signal)
    : _graph(&graph), _signal(signal), _logger(gsp::logging::getLogger("GraphSignal")) {
    if (_graph->num_nodes != _signal.size()) {
        const std::string msg = fmt::format("Signal size {} does not match graph size {}", _signal.size(), _graph->num_nodes);
        _logger->error(msg);
        throw std::length_error(msg);
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