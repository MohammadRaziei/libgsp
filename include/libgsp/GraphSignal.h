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

#include <functional>
#include <optional>


namespace gsp {



// SignalMask: one-dimensional boolean mask stored as sparse complement.
// Meaning: "true" values are implicit (default), "false" values are stored.
// Meaning: logical true is the default (implicit); positions that are false are stored as 1 in sparse.
class SignalMask {
public:
    using SparseComplementMask = Eigen::SparseVector<uint8_t>; // 1 where mask == false
    using DenseMask            = Eigen::VectorX<uint8_t>;       // each entry: 0 or 1

    explicit SignalMask(uint32_t size = 0);
    SignalMask(const std::initializer_list<uint8_t>& vec);
    SignalMask(uint32_t size, const std::initializer_list<std::pair<uint32_t, bool>>& mask);

    void resize(uint32_t size);

    // Element access (bool API)
    void set(uint32_t idx, bool value);   // value=true  -> remove from complement; value=false -> store 1 in complement
    bool get(uint32_t idx) const;         // returns true if NOT present in complement

    // Set entire mask from a dense 0/1 vector (1=true, 0=false)
    void setMask(const DenseMask& mask);


    // Set directly from sparse complement (1 where mask=false)
    void setComplementMask(const SparseComplementMask& complement);


    // Getters
    [[nodiscard]] const SparseComplementMask& complement() const { return _sparse_complement; }
    [[nodiscard]] uint32_t size() const { return _size; }

private:
    uint32_t _size{0};
    SparseComplementMask _sparse_complement;
    gsp::logging::Logger _logger;
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

    Signal(const VectorT& vec)
        : _logger(gsp::logging::getLogger("Signal")),
        _signal(vec), _mask(vec.size()) {}

    Signal(const std::initializer_list<T>& vec)
        : _logger(gsp::logging::getLogger("Signal")),
         _signal(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
        _mask(vec.size()) {}

    Signal(const std::initializer_list<T>& vec, const std::initializer_list<std::pair<uint32_t, bool>>& mask)
       : _logger(gsp::logging::getLogger("Signal")),
        _signal(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
        _mask(static_cast<uint32_t>(vec.size()), mask) {}

    Signal(const std::initializer_list<T>& vec, const std::initializer_list<uint8_t>& mask)
        : _logger(gsp::logging::getLogger("Signal")),
        _signal(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
        _mask(mask) {
        if (mask.size() != _mask.size()) {
            const std::string msg = fmt::format("Signal size mismatch: {} != {}", vec.size(), mask.size());
            _logger->error(msg);
            throw std::invalid_argument(msg);
        }
    }

    Signal(const std::vector<T>& vec)
        : _signal(Eigen::Map<const VectorT>(vec.data(), vec.size())), _mask(vec.size()) {}

    Signal(const std::vector<std::optional<T>>& vec_opt) {
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

    void setMask(uint32_t idx, bool value) {
        _mask.set(idx, value);
    }

    void setComplementMask(const SignalMask::SparseComplementMask& m) {
        _mask.setComplementMask(m);
    }

    const SignalMask& mask() const { return _mask; }

    // vector API
    const VectorT& signal() const { return _signal; }
    VectorT& signal() { return _signal; }

    std::vector<std::optional<T>> vector() const {
        std::vector<std::optional<T>> out(size());
        for (int i = 0; i < size(); ++i) {
            out[i] = _mask.get(i) ? std::optional<T>(_signal(i)) : std::nullopt;
        }
        return out;
    }

    // element API
    Signal& set(int idx, const std::optional<T>& value) {
        if (value) {
            _signal(idx) = *value;
        } else {
            _mask.set(idx, false);
            _signal(idx) = T{};
        }
        return *this;
    }

    std::optional<T> get(int idx) const {
        return _mask.get(idx) ? std::optional<T>(_signal(idx)) : std::nullopt;
    }

    // applyMask masked-out elements (set them to zero)
    Signal<T>& applyMask() {
        for (int i = 0; i < size(); ++i) {
            if (!_mask.get(i)) {
                _signal(i) = T{};
            }
        }
        return *this;
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
    gsp::logging::Logger _logger;
};



template <class Matrix, class T>
class GraphSignal {
   public:
    GraphSignal(gsp::Graph<Matrix>& graph, const gsp::Signal<T>& signal)
        : _graph(&graph), _signal(signal), _logger(gsp::logging::getLogger("GraphSignal")) {
        if (_graph->num_nodes != _signal.size()) {
            const std::string msg = fmt::format("Signal size {} does not match graph size {}", _signal.size(), _graph->num_nodes);
            _logger->error(msg);
            throw std::length_error(msg);
        }
    }

    gsp::Graph<Matrix>& graph() const { return *_graph; }
    gsp::Signal<T>& signal() { return _signal; }


   private:
    gsp::Graph<Matrix>* _graph;
    gsp::Signal<T> _signal;
    gsp::logging::Logger _logger;
};
} // namespace gsp



#endif  // LIBGSP_GRAPHSIGNAL_H
