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
        if (size_ != static_cast<uint32_t>(A.cols())) {
            throw std::invalid_argument("SignalMask::imul_structure(sparse): size mismatch with A.cols()");
        }

        SparseComplementMask new_comp(A.rows());

        // Iterate only invalid inputs (1 in complement means false column)
        for (SparseComplementMask::InnerIterator it(sparse_complement_); it; ++it) {
            const int col = it.index();
            for (typename Eigen::SparseMatrix<Scalar>::InnerIterator jt(A, col); jt; ++jt) {
                if (jt.value() != Scalar(0)) {
                    // directly insert row as false in complement
                    new_comp.insertBack(jt.row()) = uint8_t(1);
                }
            }
        }

        new_comp.finalize();
        size_ = static_cast<uint32_t>(A.rows());
        sparse_complement_ = std::move(new_comp);
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
    [[nodiscard]] const SparseComplementMask& complement() const { return sparse_complement_; }
    [[nodiscard]] uint32_t size() const {  return size_; }

    [[nodiscard]] uint32_t nnz() const;

private:
    uint32_t size_{0};
    SparseComplementMask sparse_complement_;
    gsp::logging::Logger logger_;
};


template <typename T>
class Signal {
public:
    using VectorT = Eigen::Matrix<T, Eigen::Dynamic, 1>;
    using type = T;

    // --- Constructors ---
    explicit Signal(int size = 0)
        : logger_(gsp::logging::getLogger("Signal")),
          signal_(size), mask_(size) {
        signal_.setZero();
    }

    Signal(const VectorT& vec)
        : logger_(gsp::logging::getLogger("Signal")),
          signal_(vec), mask_(vec.size()) {}

    Signal(const VectorT& vec, const SignalMask& mask)
        : logger_(gsp::logging::getLogger("Signal")),
          signal_(vec), mask_(mask) {
            if (mask.size() != vec.size()) {
                const std::string msg = fmt::format("Signal size mismatch: {} != {}", vec.size(), mask.size());
                logger_->error(msg);
                throw std::invalid_argument(msg);
            }
            applyMask();
        }

    Signal(const std::initializer_list<T>& vec)
        : logger_(gsp::logging::getLogger("Signal")),
          signal_(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
          mask_(vec.size()) {}

    Signal(const std::initializer_list<T>& vec, const std::initializer_list<std::pair<uint32_t, bool>>& mask)
       : logger_(gsp::logging::getLogger("Signal")),
         signal_(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
         mask_(static_cast<uint32_t>(vec.size()), mask) {
        applyMask();
    }

    Signal(const std::initializer_list<T>& vec, const std::initializer_list<uint8_t>& mask)
        : logger_(gsp::logging::getLogger("Signal")),
          signal_(Eigen::Map<const VectorT>(vec.begin(), static_cast<uint32_t>(vec.size()))),
          mask_(mask) {
        if (mask.size() != mask_.size()) {
            const std::string msg = fmt::format("Signal size mismatch: {} != {}", vec.size(), mask.size());
            logger_->error(msg);
            throw std::invalid_argument(msg);
        }
        applyMask();
    }

    Signal(const std::vector<T>& vec)
        : signal_(Eigen::Map<const VectorT>(vec.data(), vec.size())), mask_(vec.size()) {}

    Signal(const std::vector<std::optional<T>>& vec_opt) {
        resize(static_cast<int>(vec_opt.size()));
        for (int i = 0; i < size(); ++i) {
            if (vec_opt[i].has_value()) {
                signal_(i) = *vec_opt[i];
                // _mask.set(i, true);
            } else {
                signal_(i) = T{};
                mask_.set(i, false);
            }
        }
    }

    // --- Core API ---
    void resize(int n) {
        signal_.resize(n);
        signal_.setZero();
        mask_.resize(n);
    }

    [[nodiscard]] uint32_t size() const { return static_cast<uint32_t>(signal_.size()); }

    // mask API
    void setMask(SignalMask m) {
        if (m.size() != size()) {
            const std::string msg = fmt::format("setMask: size mismatch {} != {}", size(), m.size());
            logger_->error(msg);
            throw std::invalid_argument(msg);
        }
        mask_ = std::move(m);
    }

    void setMask(uint32_t idx, bool value) {
        mask_.set(idx, value);
    }

    void setComplementMask(const SignalMask::SparseComplementMask& m) {
        mask_.setComplementMask(m);
    }

    [[nodiscard]] const SignalMask& mask() const { return mask_; }

    [[nodiscard]] bool mask(uint32_t idx) const { return mask_.at(idx); }


    const double& operator[](uint32_t idx) const {
        if (idx >= size()) {
            throw std::out_of_range("idx is out of range");
        }
        return signal_[idx];
    }

    double& operator[](uint32_t idx) {
        if (idx >= size()) {
            throw std::out_of_range("idx is out of range");
        }
        return signal_[idx];
    }

    // vector API
    const VectorT& signal() const { return signal_; }
    VectorT& signal() { return signal_; }



    T signal(uint32_t idx) const { return signal_(idx); }
    T& signal(uint32_t idx) { return signal_(idx); }


    // Dereference operator
    VectorT& operator*() {
        return signal_;
    }
    const VectorT& operator*() const {
        return signal_;
    }
    // Pointer operator
    VectorT* operator->() {
        return &signal_;
    }
    const VectorT* operator->() const {
        return &signal_;
    }


    std::vector<std::optional<T>> vector() const {
        std::vector<std::optional<T>> out(size());
        for (int i = 0; i < size(); ++i) {
            out[i] = mask_.at(i) ? std::optional<T>(signal_(i)) : std::nullopt;
        }
        return out;
    }

    // element API
    Signal& set(int idx, const std::optional<T>& value) {
        if (value) {
            signal_(idx) = *value;
            mask_.set(idx, true);
        } else {
            mask_.set(idx, false);
            signal_(idx) = T{};
        }
        return *this;
    }

    std::optional<T> get(int idx) const {
        return mask_.at(idx) ? std::optional<T>(signal_(idx)) : std::nullopt;
    }

    Signal<T> mul(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& M) const {
        return Signal<T>(M * signal_, mask_.mul(M));
    }

    // Out-of-place multiply (sparse)
    Signal<T> mul(const Eigen::SparseMatrix<T>& M) const {
        return Signal<T>(M * signal_, mask_.mul(M));
    }

    // In-place multiply (dense)
    Signal<T>& imul(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& M) {
        signal_ = M * signal_;
        mask_.imul(M);
        return applyMask();
    }

    // In-place multiply (sparse)
    Signal<T>& imul(const Eigen::SparseMatrix<T>& M) {
        signal_ = M * signal_;
        mask_.imul(M);
        return applyMask();
    }


    Signal<T> operator*(const Signal<T>& other) const {
        VectorT out(signal_.array() * other.signal_.array());
        return Signal<T>(out, mask_ + other.mask_);
    }

    Signal<T> operator+(const Signal<T>& other) const {
        return Signal<T>(signal_ + other.signal_, mask_ + other.mask_);
    }

    Signal<T> operator-(const Signal<T>& other) const {
        return Signal<T>(signal_ - other.signal_, mask_ + other.mask_);
    }

    Signal<T> operator/(const Signal<T>& other) const {
        return Signal<T>(signal_ / other.signal_, mask_ + other.mask_);
    }

    Signal<T>& operator*=(const Signal<T>& other) {
        signal_ *= other.signal_;
        mask_ += other.mask_;
        return applyMask();
    }

    Signal<T>& operator+=(const Signal<T>& other) {
        signal_ += other.signal_;
        mask_ += other.mask_;
        return applyMask();
    }

    Signal<T>& operator-=(const Signal<T>& other) {
        signal_ -= other.signal_;
        mask_ += other.mask_;
        return applyMask();
    }

    Signal<T>& operator/=(const Signal<T>& other) {
        signal_ /= other.signal_;
        mask_ += other.mask_;
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
            if (mask_.at(i)) {
                sig_out(i) = func(signal_(i));
            }
        }
        return Signal<outT>(sig_out, mask_);
    }

    template <typename Func>
    Signal<T>& applyInplace(const Func& func) {
        static_assert(std::is_same_v<T, typename std::invoke_result_t<Func, const T&>>);
        for (int i = 0; i < size(); ++i) {
            if (mask_.at(i)) {
                signal_(i) = func(signal_(i));
            }
        }
        return *this;
    }

    // applyMask masked-out elements (set them to zero)
    Signal<T>& applyMask() {
        for (int i = 0; i < size(); ++i) {
            if (!mask_.at(i)) {
                signal_(i) = T{};
            }
        }
        return *this;
    }

    Signal<T> compressed() const {
        Signal<T> out(mask_.nnz());
        for (uint32_t i = 0, j = 0; i < size(); ++i) {
            if (mask_.at(i)) {
                out.set(j++, signal_(i));
            }
        }
        return out;
    }

    // pretty-print
    [[nodiscard]] std::string str() const {
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), "[");
        for (int i = 0; i < size(); ++i) {
            fmt::format_to(std::back_inserter(buf), mask_.at(i) ? fmt::format("{}", signal_(i)) : "n");
            if (i + 1 < size()) fmt::format_to(std::back_inserter(buf), ", ");
        }
        fmt::format_to(std::back_inserter(buf), "]'");
        return fmt::to_string(buf);
    }

private:
    VectorT signal_;
    SignalMask mask_;
    gsp::logging::Logger logger_;
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
