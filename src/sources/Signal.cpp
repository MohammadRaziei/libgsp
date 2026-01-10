//
// Created by Mohammad on 7/22/2025.
//

#include "libgsp/Signal.h"
#include "libgsp/utils/Logging.h"

using namespace gsp;


SignalMask::SignalMask(uint32_t size) : logger_(gsp::logging::getLogger("SignalMask")) {
    resize(size);
}

SignalMask::SignalMask(const std::initializer_list<uint8_t>& vec)
    : logger_(gsp::logging::getLogger("SignalMask")) {
    setMask(Eigen::Map<const DenseMask>(vec.begin(), static_cast<uint32_t>(vec.size())));
}

SignalMask::SignalMask(uint32_t size, const std::initializer_list<std::pair<uint32_t, bool>>& mask)
    : logger_(gsp::logging::getLogger("SignalMask")){
    resize(size);
    for (const auto& [idx, value] : mask) {
        if (idx >= size) {
            std::string msg = fmt::format("Invalid index {} in initializer list", idx);
            logger_->error(msg);
            throw std::out_of_range(msg);
        }
        if (!value) sparse_complement_.coeffRef(idx) = static_cast<uint8_t>(1);
    }
}


void SignalMask::resize(uint32_t size) {
    size_ = size;
    sparse_complement_.resize(size);
    sparse_complement_.setZero();
    // Note: no need to prune now; vector is empty
}

void SignalMask::set(uint32_t idx, bool value) {
    if (idx >= size_) {
        std::string msg = fmt::format("Invalid index {}", idx);
        logger_->error(msg);
        throw std::out_of_range(msg);
    }

    if (value) {
        // logical true -> default -> ensure complement at idx is 0
        if (sparse_complement_.coeff(idx)) {
            sparse_complement_.coeffRef(idx) = static_cast<uint8_t>(0);
        }
    } else {
        // logical false -> store 1 in complement
        sparse_complement_.coeffRef(idx) = static_cast<uint8_t>(1);
    }
}

bool SignalMask::at(uint32_t idx) const {
    if (idx >= size_) {
        std::string msg = fmt::format("Invalid index {}", idx);
        logger_->error(msg);
        throw std::out_of_range(msg);
    }
    // true iff not present (or zero) in complement
    return sparse_complement_.coeff(idx) == static_cast<uint8_t>(0);
}

void SignalMask::setMask(const DenseMask& mask) {
    resize(static_cast<int>(mask.size()));

    // complement = (mask == 0) ? 1 : 0  (vectorized)
    // Cast to integer type explicitly to avoid bool specialization pitfalls
    Eigen::VectorX<uint8_t> complement = (mask.array() == static_cast<uint8_t>(0))
                                           .template cast<uint8_t>();

    // Build sparse directly from dense 0/1
    sparse_complement_ = complement.sparseView(0 /*reference*/, 1 /*epsilon*/);
    // The epsilon=1 here is fine because entries are exactly 0 or 1 (nonzero threshold).
    // If you prefer the default, simply use: complement.sparseView();
}



void SignalMask::setComplementMask(const SparseComplementMask& complement) {
    resize(static_cast<int>(complement.size()));
    sparse_complement_ = complement;  // shallow copy of sparse structure/content
}

uint32_t SignalMask::nnz() const {
    return size_ - sparse_complement_.cast<uint32_t>().sum();
}



SignalMask SignalMask::operator+(const SignalMask& other) const {
    SignalMask out(*this);
    return out += other;
}

SignalMask& SignalMask::operator+=(const SignalMask& other) {
    if (other.size() != size()) {
        const std::string msg = fmt::format("size mismatched");
        logger_->error(msg);
        throw std::invalid_argument(msg);
    }

    for (SparseComplementMask::InnerIterator it2(other.sparse_complement_);
         it2; ++it2) {
        sparse_complement_.coeffRef(it2.index()) = static_cast<uint8_t>(1);
         }
    return *this;
}


std::string SignalMask::str() const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "[");
    for (uint32_t i = 0; i < size_; ++i) {
        fmt::format_to(std::back_inserter(buf), at(i) ? "1" : "n");
        if (i + 1 < size_) {
            fmt::format_to(std::back_inserter(buf), ", ");
        }
    }
    fmt::format_to(std::back_inserter(buf), "]");
    return fmt::to_string(buf);
}