//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/Signal.h"
#include "libgsp/utils/Logging.h"

using namespace gsp;


SignalMask::SignalMask(uint32_t size) : _logger(gsp::logging::getLogger("SignalMask")) {
    resize(size);
}

SignalMask::SignalMask(const std::initializer_list<uint8_t>& vec)
    : _logger(gsp::logging::getLogger("SignalMask")) {
    setMask(Eigen::Map<const DenseMask>(vec.begin(), static_cast<uint32_t>(vec.size())));
}

SignalMask::SignalMask(uint32_t size, const std::initializer_list<std::pair<uint32_t, bool>>& mask)
    : _logger(gsp::logging::getLogger("SignalMask")){
    resize(size);
    for (const auto& [idx, value] : mask) {
        if (idx >= size) {
            std::string msg = fmt::format("Invalid index {} in initializer list", idx);
            _logger->error(msg);
            throw std::out_of_range(msg);
        }
        if (!value) _sparse_complement.coeffRef(idx) = static_cast<uint8_t>(1);
    }
}


void SignalMask::resize(uint32_t size) {
    _size = size;
    _sparse_complement.resize(size);
    _sparse_complement.setZero();
    // Note: no need to prune now; vector is empty
}

void SignalMask::set(uint32_t idx, bool value) {
    if (idx >= _size) {
        std::string msg = fmt::format("Invalid index {}", idx);
        _logger->error(msg);
        throw std::out_of_range(msg);
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

bool SignalMask::at(uint32_t idx) const {
    if (idx >= _size) {
        std::string msg = fmt::format("Invalid index {}", idx);
        _logger->error(msg);
        throw std::out_of_range(msg);
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

uint32_t SignalMask::nnz() const {
    return _size - static_cast<uint32_t>(_sparse_complement.nonZeros());
}

