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

    template <typename Scalar>
    SignalMask& imul(const Eigen::SparseMatrix<Scalar>& A) {
        if (_size != static_cast<uint32_t>(A.cols())) {
            throw std::invalid_argument("SignalMask::imul_structure(sparse): size mismatch with A.cols()");
        }

        SparseComplementMask new_comp(A.rows());

        // Iterate only invalid inputs (1 in complement means false column)
        for (SparseComplementMask::InnerIterator it(_sparse_complement); it; ++it) {
            const int col = it.index();
            for (typename Eigen::SparseMatrix<Scalar>::InnerIterator jt(A, col); jt; ++jt) {
                if (jt.value() != Scalar(0)) {
                    // directly insert row as false in complement
                    new_comp.insertBack(jt.row()) = uint8_t(1);
                }
            }
        }

        new_comp.finalize();
        _size = static_cast<uint32_t>(A.rows());
        _sparse_complement = std::move(new_comp);
        return *this;
    }

    template <typename Scalar>
    SignalMask& imul(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& A) {
        const Eigen::SparseMatrix<Scalar> A_sparse = A.sparseView(1e-12);
        return imul(A_sparse);
    }

    template <typename Scalar>
    SignalMask mul(const Eigen::SparseMatrix<Scalar>& A) const {
        SignalMask out = *this;
        out.imul(A);
        return out;
    }

    template <typename Scalar>
    SignalMask mul(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& A) const {
        SignalMask out = *this;
        out.imul(A);
        return out;
    }

    std::string str() const;

    // Getters
    [[nodiscard]] const SparseComplementMask& complement() const { return _sparse_complement; }
    [[nodiscard]] uint32_t size() const {  return _size; }

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

    [[nodiscard]] const SignalMask& mask() const { return _mask; }

    [[nodiscard]] bool mask(uint32_t idx) const { return _mask.at(idx); }


    // vector API
    const VectorT& signal() const { return _signal; }
    VectorT& signal() { return _signal; }

    T signal(uint32_t idx) const { return _signal(idx); }
    T& signal(uint32_t idx) { return _signal(idx); }


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

    Signal<T> mul(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& M) const {
        auto y = M * _signal;                 // numeric result
        auto newMask = _mask.mul(M); // structural mask: cols -> rows
        return Signal<T>(y, newMask).applyMask();
    }

    // Out-of-place multiply (sparse)
    Signal<T> mul(const Eigen::SparseMatrix<T>& M) const {
        auto y = M * _signal;
        auto newMask = _mask.mul(M);
        return Signal<T>(y, newMask).applyMask();
    }

    // In-place multiply (dense)
    Signal<T>& imul(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& M) {
        _signal = M * _signal;
        _mask.imul(M);
        return applyMask();
    }

    // In-place multiply (sparse)
    Signal<T>& imul(const Eigen::SparseMatrix<T>& M) {
        _signal = M * _signal;
        _mask.imul(M);
        return applyMask();
    }


    Signal<T> operator*(const Signal<T>& other) const {
        return Signal<T>(_signal * other._signal, _mask + other._mask).applyMask();
    }

    Signal<T> operator+(const Signal<T>& other) const {
        return Signal<T>(_signal + other._signal, _mask + other._mask).applyMask();
    }

    Signal<T> operator-(const Signal<T>& other) const {
        return Signal<T>(_signal - other._signal, _mask + other._mask).applyMask();
    }

    Signal<T> operator/(const Signal<T>& other) const {
        return Signal<T>(_signal / other._signal, _mask + other._mask).applyMask();
    }

    Signal<T>& operator*=(const Signal<T>& other) {
        _signal *= other._signal;
        _mask += other._mask;
        return applyMask();
    }

    Signal<T>& operator+=(const Signal<T>& other) {
        _signal += other._signal;
        _mask += other._mask;
        return applyMask();
    }

    Signal<T>& operator-=(const Signal<T>& other) {
        _signal -= other._signal;
        _mask += other._mask;
        return applyMask();
    }

    Signal<T>& operator/=(const Signal<T>& other) {
        _signal /= other._signal;
        _mask += other._mask;
        return applyMask();
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
                sig_out(i) = func(_signal(i));
            }
        }
        return Signal<outT>(sig_out, _mask);
    }

    template <typename Func>
    Signal<T>& applyInplace(const Func& func) {
        static_assert(std::is_same_v<T, typename std::invoke_result_t<Func, const T&>>);
        for (int i = 0; i < size(); ++i) {
            if (_mask.at(i)) {
                _signal(i) = func(_signal(i));
            }
        }
        return *this;
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
        fmt::format_to(std::back_inserter(buf), "]'");
        return fmt::to_string(buf);
    }

private:
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
