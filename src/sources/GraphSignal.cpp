//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/GraphSignal.h"

#include "libgsp/utils/Logging.h"

using namespace gsp;


SignalMask::SignalMask(int size) {
    resize(size);
}

void SignalMask::resize(int size) {
    _size = size;
    _sparse_complement.resize(size);
    _sparse_complement.setZero();
    // Note: no need to prune now; vector is empty
}

void SignalMask::set(int idx, bool value) {
    if (idx < 0 || idx >= _size) {
        throw std::out_of_range("SignalMask::set index out of range");
    }

    if (value) {
        // logical true -> default -> ensure complement at idx is 0
        if (_sparse_complement.coeff(idx)) {
            _sparse_complement.coeffRef(idx) = static_cast<uint8_t>(0);
            // Optional: prune zeros lazily; not strictly required.
            // _sparse_complement.prune(0);
        }
    } else {
        // logical false -> store 1 in complement
        _sparse_complement.coeffRef(idx) = static_cast<uint8_t>(1);
    }
}

bool SignalMask::get(int idx) const {
    if (idx < 0 || idx >= _size) {
        throw std::out_of_range("SignalMask::get index out of range");
    }
    // true iff not present (or zero) in complement
    return _sparse_complement.coeff(idx) == static_cast<uint8_t>(0);
}

void SignalMask::setMask(const DenseMask& mask) {
    resize(static_cast<int>(mask.size()));

    // complement = (mask == 0) ? 1 : 0  (vectorized)
    // Cast to integer type explicitly to avoid bool specialization pitfalls
    Eigen::VectorX<uint8_t> complement = (mask.array() == static_cast<uint8_t>(0))
                                           .template cast<uint8_t>();

    // Build sparse directly from dense 0/1
    _sparse_complement = complement.sparseView(0 /*reference*/, 1 /*epsilon*/);
    // The epsilon=1 here is fine because entries are exactly 0 or 1 (nonzero threshold).
    // If you prefer the default, simply use: complement.sparseView();
}

void SignalMask::setComplementMask(const SparseComplementMask& complement) {
    resize(static_cast<int>(complement.size()));
    _sparse_complement = complement;  // shallow copy of sparse structure/content
}





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