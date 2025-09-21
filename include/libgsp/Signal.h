#ifndef LIBGSP_SIGNAL_H
#define LIBGSP_SIGNAL_H
#pragma once

#include <functional>
#include <optional>
#include <type_traits>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "libgsp/utils/Logging.h"

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
    bool at(uint32_t idx) const;         // returns true if NOT present in complement

    // Set entire mask from a dense 0/1 vector (1=true, 0=false)
    void setMask(const DenseMask& mask);


    // Set directly from sparse complement (1 where mask=false)
    void setComplementMask(const SparseComplementMask& complement);

    SignalMask operator+(const SignalMask& other) const;

    SignalMask& operator+=(const SignalMask& other);

    // Getters
    [[nodiscard]] const SparseComplementMask& complement() const { return _sparse_complement; }
    [[nodiscard]] uint32_t size() const { return _size; }

    [[nodiscard]] uint32_t nnz() const;

private:
    uint32_t _size{0};
    SparseComplementMask _sparse_complement;
    gsp::logging::Logger _logger;
};


template <typename T>
class Signal {
public:
    using VectorT = Eigen::Matrix<T, Eigen::Dynamic, 1>;
    using type = T;

    // --- Constructors ---
    explicit Signal(int size = 0)
        : _signal(size), _mask(size) {
        _signal.setZero();
    }

    Signal(const VectorT& vec)
        : _logger(gsp::logging::getLogger("Signal")),
        _signal(vec), _mask(vec.size()) {}

    Signal(const VectorT& vec, const SignalMask& mask)
        : _logger(gsp::logging::getLogger("Signal")),
        _signal(vec), _mask(mask) {
            if (mask.size() != vec.size()) {
                const std::string msg = fmt::format("Signal size mismatch: {} != {}", vec.size(), mask.size());
                _logger->error(msg);
                throw std::invalid_argument(msg);
            }
        }

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
            const std::string msg = fmt::format("setMask: size mismatch {} != {}", m.size(), size());
            _logger->error(msg);
            throw std::invalid_argument(msg);
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
            out[i] = _mask.at(i) ? std::optional<T>(_signal(i)) : std::nullopt;
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
        return _mask.at(idx) ? std::optional<T>(_signal(idx)) : std::nullopt;
    }

    Signal<T> mul(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& matrix) const {
        return Signal<T>(matrix * _signal, _mask);
    }

    Signal<T> mul(Eigen::SparseMatrix<T>& matrix) const {
        return Signal<T>(matrix * _signal, _mask);
    }

    Signal<T>& imul(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& matrix) {
        _signal = matrix * _signal;
        return *this;
    }

    Signal<T>& imul(Eigen::SparseMatrix<T>& matrix) {
        _signal = matrix * _signal;
        return *this;
    }


    Signal<T> operator*(const Signal<T>& other) const {
        return Signal<T>(_signal * other._signal, _mask + other._mask);
    }

    Signal<T> operator+(const Signal<T>& other) const {
        return Signal<T>(_signal + other._signal, _mask + other._mask);
    }

    Signal<T> operator-(const Signal<T>& other) const {
        return Signal<T>(_signal - other._signal, _mask + other._mask);
    }

    Signal<T> operator/(const Signal<T>& other) const {
        return Signal<T>(_signal / other._signal, _mask + other._mask);
    }

    Signal<T>& operator*=(const Signal<T>& other) {
        _signal *= other._signal;
        _mask += other._mask;
        return *this;
    }

    Signal<T>& operator+=(const Signal<T>& other) {
        _signal += other._signal;
        _mask += other._mask;
        return *this;
    }

    Signal<T>& operator-=(const Signal<T>& other) {
        _signal -= other._signal;
        _mask += other._mask;
        return *this;
    }

    Signal<T>& operator/=(const Signal<T>& other) {
        _signal /= other._signal;
        _mask += other._mask;
        return *this;
    }

    operator std::string() const {
        return str();
    }

    // Apply a unary function element-wise, preserving the mask.
    // Accepts any callable (e.g. lambda, functor, std::function).
    template <typename Func>
    auto apply(const Func& func) const -> Signal<typename std::invoke_result_t<Func, const T&>> {
        using outT = typename std::invoke_result_t<Func, const T&>;
        Eigen::Matrix<outT, Eigen::Dynamic, 1> sig_out(size());
        for (int i = 0; i < size(); ++i) {
            if (_mask.at(i)) {
                sig_out[i] = func(_signal[i]);
            }
        }
        return Signal<outT>(sig_out, _mask);
    }

    // applyMask masked-out elements (set them to zero)
    Signal<T>& applyMask() {
        for (int i = 0; i < size(); ++i) {
            if (!_mask.at(i)) {
                _signal(i) = T{};
            }
        }
        return *this;
    }

    Signal<T> compressed() const {
        Signal<T> out(_mask.nnz());
        for (uint32_t i = 0, j = 0; i < size(); ++i) {
            if (_mask.at(i)) {
                out.set(j++, _signal(i));
            }
        }
        return out;
    }

    // pretty-print
    [[nodiscard]] std::string str() const {
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), "[");
        for (int i = 0; i < size(); ++i) {
            fmt::format_to(std::back_inserter(buf), _mask.at(i) ? fmt::format("{}", _signal(i)) : "n");
            if (i + 1 < size()) fmt::format_to(std::back_inserter(buf), ", ");
        }
        fmt::format_to(std::back_inserter(buf), "]");
        return fmt::to_string(buf);
    }

private:
    // template <typename outT>
    // friend gsp::Signal<outT> gsp::Signal<T>::apply(std::function<outT(T)>);
    VectorT _signal;
    SignalMask _mask;
    gsp::logging::Logger _logger;
};

template <typename T, typename Func>
auto arrayfun(const gsp::Signal<T>& signal, Func&& func)
    -> gsp::Signal<std::invoke_result_t<std::decay_t<Func>, const T&>> {
    return signal.apply(std::forward<Func>(func));
}


namespace function {
template <typename T>
double todouble(T value) {
    return static_cast<double>(value);
}
}

} // namespace gsp

template <typename T>
gsp::Signal<T> operator*(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& matrix, const gsp::Signal<T>& signal) {
    return signal.mul(matrix);
}
template <typename T>
gsp::Signal<T> operator*(const Eigen::SparseMatrix<T>& matrix, const gsp::Signal<T>& signal) {
    return signal.mul(matrix);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const gsp::Signal<T>& signal) {
    os << signal.str();
    return os;
}




#endif  // LIBGSP_SIGNAL_H
