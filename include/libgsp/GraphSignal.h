//
// Created by Mohammad on 7/22/2025.
//

#ifndef LIBGSP_GRAPHSIGNAL_H
#define LIBGSP_GRAPHSIGNAL_H
#pragma once


#include "Graph.h"
#include "libgsp/utils/Logging.h"

// #include "libgsp/utils/types.h"
#include <Eigen/Sparse>
#include <Eigen/Dense>




namespace gsp {



// SignalMask: one-dimensional boolean mask stored as sparse complement.
// Meaning: "true" values are implicit (default), "false" values are stored.
// Meaning: logical true is the default (implicit); positions that are false are stored as 1 in sparse.
class SignalMask {
public:
    using SparseComplementMask = Eigen::SparseVector<uint8_t>; // 1 where mask == false
    using DenseMask            = Eigen::VectorX<uint8_t>;       // each entry: 0 or 1

    explicit SignalMask(int size = 0);

    void resize(int size);

    // Element access (bool API)
    void set(int idx, bool value);   // value=true  -> remove from complement; value=false -> store 1 in complement
    bool get(int idx) const;         // returns true if NOT present in complement

    // Set entire mask from a dense 0/1 vector (1=true, 0=false)
    void setMask(const DenseMask& mask);

    // Set directly from sparse complement (1 where mask=false)
    void setComplementMask(const SparseComplementMask& complement);

    // Getters
    const SparseComplementMask& complement() const { return _sparse_complement; }
    int size() const { return _size; }

private:
    int _size{0};
    SparseComplementMask _sparse_complement;
};




template <typename T>
class Signal {
public:
    using VectorT = Eigen::Matrix<T, Eigen::Dynamic, 1>;

    // --- Constructors ---
    explicit Signal(int size = 0)
        : _signal(size), _mask(size) {
        _signal.setZero();
    }

    explicit Signal(const VectorT& vec)
        : _signal(vec), _mask(vec.size()) {}

    explicit Signal(const std::vector<std::optional<T>>& vec_opt) {
        resize(static_cast<int>(vec_opt.size()));
        for (int i = 0; i < size(); ++i) {
            if (vec_opt[i].has_value()) {
                _signal(i) = *vec_opt[i];
                // _mask.set(i, true);
            } else {
                _signal(i) = T{};
                _mask.set(i, false);
            }
        }
    }

    // --- Core API ---
    void resize(int n) {
        _signal.resize(n);
        _signal.setZero();
        _mask.resize(n);
    }

    int size() const { return static_cast<int>(_signal.size()); }

    // mask API
    void setMask(SignalMask m) {
        if (m.size() != size()) {
            throw std::invalid_argument("Signal::setMask: size mismatch");
        }
        _mask = std::move(m);
    }
    const SignalMask& mask() const { return _mask; }

    // vector API
    const VectorT& signal() const { return _signal; }
    VectorT& signal() { return _signal; }

    std::vector<std::optional<T>> vector() const {
        std::vector<std::optional<T>> out(size());
        for (int i = 0; i < size(); ++i) {
            if (_mask.get(i)) {
                out[i] = _signal(i);
            } else {
                out[i] = std::nullopt;
            }
        }
        return out;
    }

    // element API
    void set(int idx, const T& value) {
        _signal(idx) = value;
        _mask.set(idx, true);
    }

    std::optional<T> get(int idx) const {
        return _mask.get(idx) ? std::optional<T>(_signal(idx)) : std::nullopt;
    }

    // flush masked-out elements (set them to zero)
    void flush() {
        for (int i = 0; i < size(); ++i) {
            if (!_mask.get(i)) {
                _signal(i) = T{};
            }
        }
    }

    // pretty-print
    [[nodiscard]] std::string str() const {
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), "[");
        for (int i = 0; i < size(); ++i) {
            if (_mask.get(i)) {
                fmt::format_to(std::back_inserter(buf), "{}", _signal(i));
            } else {
                fmt::format_to(std::back_inserter(buf), "α");
            }
            if (i + 1 < size()) fmt::format_to(std::back_inserter(buf), ", ");
        }
        fmt::format_to(std::back_inserter(buf), "]");
        return fmt::to_string(buf);
    }

private:
    VectorT _signal;
    SignalMask _mask;
};



template <class Matrix, class Signal>
class GraphSignal {
   public:
    GraphSignal(gsp::Graph<Matrix>& graph,
                const Signal& signal);

   public:
    gsp::Graph<Matrix>* _graph;
    Signal _signal;
    gsp::logging::Logger _logger;
};
} // namespace gsp



#endif  // LIBGSP_GRAPHSIGNAL_H
