//
// Created by Mohammad on 7/24/2025
//

#ifndef LIBGSP_MATRIX_H
#define LIBGSP_MATRIX_H
#pragma once

#include <cstdint>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unsupported/Eigen/KroneckerProduct>

#include "libgsp/utils/Types.h"

namespace gsp::matrix {
// ---------- allocate ----------
template<typename Scalar, int Options, typename Index>
void allocate(Eigen::SparseMatrix<Scalar, Options, Index>& m,
              uint32_t rows, uint32_t cols) {
    m.resize(static_cast<Eigen::Index>(rows),
             static_cast<Eigen::Index>(cols));
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void allocate(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
              uint32_t rows, uint32_t cols) {
    m.resize(static_cast<Eigen::Index>(rows),
             static_cast<Eigen::Index>(cols));
}

// ---------- fillZero ----------
template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void fillZero(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& matrix) {
    matrix.setZero();
}

template<typename Scalar, int Options, typename Index>
void fillZero(Eigen::SparseMatrix<Scalar, Options, Index>& matrix) {
    matrix.setZero();
}

// ---------- free ----------
template<typename Scalar, int Options, typename Index>
void free(Eigen::SparseMatrix<Scalar, Options, Index>& m) {
    m.resize(0, 0);
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void free(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m) {
    m.resize(0, 0);
}

// ---------- setElement ----------
template<typename Scalar, int Options, typename Index>
void setElement(Eigen::SparseMatrix<Scalar, Options, Index>& m,
                uint32_t r, uint32_t c, Scalar el) {
    if (m.isCompressed()) m.uncompress();
    m.coeffRef(static_cast<Eigen::Index>(r),
               static_cast<Eigen::Index>(c)) = el;
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void setElement(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
                uint32_t r, uint32_t c, Scalar el) {
    m(static_cast<Eigen::Index>(r),
      static_cast<Eigen::Index>(c)) = el;
}

// ---------- getElement ----------
template<typename Scalar, int Options, typename Index>
Scalar getElement(Eigen::SparseMatrix<Scalar, Options, Index>& m,
                  uint32_t r, uint32_t c) {
    return m.coeff(static_cast<Eigen::Index>(r),
                   static_cast<Eigen::Index>(c));
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
Scalar getElement(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
                  uint32_t r, uint32_t c) {
    return m(static_cast<Eigen::Index>(r),
             static_cast<Eigen::Index>(c));
}


template<typename Scalar, int Options, typename Index>
Eigen::SparseMatrix<Scalar, Options, Index> kron(const Eigen::SparseMatrix<Scalar, Options, Index>& a,
                                                 const Eigen::SparseMatrix<Scalar, Options, Index>& b) {
    return Eigen::KroneckerProductSparse(a, b);
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC> kron(const Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& a,
                                                        const Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& b) {
    return Eigen::KroneckerProduct(a, b);
}


template<typename Scalar, int Options, typename Index>
Eigen::SparseMatrix<Scalar, Options, Index>
kronSum(const Eigen::SparseMatrix<Scalar, Options, Index>& A,
        const Eigen::SparseMatrix<Scalar, Options, Index>& B)
{
    using SpMat = Eigen::SparseMatrix<Scalar, Options, Index>;

    // Sparse identity matrices (do NOT use dense identity here).
    SpMat Ia(A.rows(), A.rows()); Ia.setIdentity();

    SpMat Ib(B.rows(), B.rows()); Ib.setIdentity();

    SpMat K = Eigen::kroneckerProduct(A, Ib).eval()
              + Eigen::kroneckerProduct(Ia, B).eval();

    return K;
}


template <typename DerivedA, typename DerivedB>
auto kronSum(const Eigen::MatrixBase<DerivedA>& A,
             const Eigen::MatrixBase<DerivedB>& B)
-> Eigen::Matrix<typename DerivedA::Scalar, Eigen::Dynamic, Eigen::Dynamic>
{
    using Scalar = typename DerivedA::Scalar;
    static_assert(std::is_same_v<Scalar, typename DerivedB::Scalar>,
                  "A and B must have the same scalar type.");

    // Kronecker sum is defined for square matrices
    assert(A.rows() == A.cols());
    assert(B.rows() == B.cols());

    const Eigen::Index n = A.rows();
    const Eigen::Index m = B.rows();

    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Ia(n, n);
    Ia.setIdentity();

    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Ib(m, m);
    Ib.setIdentity();

    return (Eigen::kroneckerProduct(A.derived(), Ib).eval()
            + Eigen::kroneckerProduct(Ia, B.derived()).eval());
}

template<typename Scalar, int Options, typename Index>
void transposeInplace(Eigen::SparseMatrix<Scalar, Options, Index>& mat) {
    mat = mat.transpose();
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void transposeInplace(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& mat) {
    mat.transposeInPlace();
}

// Implementation for Dense matrices
template <typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
Scalar maxCoeff(const Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>& mat) {
    return mat.maxCoeff();
}

// Implementation for Sparse matrices
template <typename Scalar, int Options, typename StorageIndex>
Scalar maxCoeff(const Eigen::SparseMatrix<Scalar, Options, StorageIndex>& mat) {
    Scalar max_val = std::numeric_limits<Scalar>::lowest();
    for (int k = 0; k < mat.outerSize(); ++k) {
        for (typename Eigen::SparseMatrix<Scalar, Options, StorageIndex>::InnerIterator it(mat, k); it; ++it) {
            if (it.value() > max_val) {
                max_val = it.value();
            }
        }
    }
    return max_val;
}





/**
 * @brief Applies a function element-wise to a Dense matrix in-place.
 *
 * @tparam Derived The type of the input matrix (e.g., MatrixXd).
 * @tparam Func The type of the callable function (lambda, functor, etc.).
 * @tparam Args Variadic types for additional arguments to pass to the function.
 * @param matrix Input dense matrix (modified in-place).
 * @param func The function to apply: Scalar func(Scalar, Args...).
 * @param args Additional arguments to forward to the function.
 */
    template <typename Derived, typename Func, typename... Args>
    void arrayfunInplace(Eigen::MatrixBase<Derived>& matrix, Func&& func, Args&&... args) {
        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<typename Derived::Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        // Efficiently iterate over the data using Eigen's raw data access
        // Note: We use .derived() to access the actual object type
        auto* data_ptr = matrix.derived().data();
        const auto size = matrix.derived().size();

        for (Eigen::Index i = 0; i < size; ++i) {
            data_ptr[i] = func(data_ptr[i], std::forward<Args>(args)...);
        }
    }

/**
 * @brief Applies a function element-wise to a Sparse matrix in-place.
 *
 * This function iterates only over non-zero elements, preserving sparsity.
 *
 * @tparam Scalar The scalar type.
 * @tparam Options The storage options (ColMajor, RowMajor).
 * @tparam StorageIndex The index type.
 * @tparam Func The type of the callable function.
 * @tparam Args Variadic types for additional arguments.
 * @param matrix Input sparse matrix (modified in-place).
 * @param func The function to apply: Scalar func(Scalar, Args...).
 * @param args Additional arguments to forward to the function.
 */
    template <typename Scalar, int Options, typename StorageIndex, typename Func, typename... Args>
    void arrayfunInplace(Eigen::SparseMatrix<Scalar, Options, StorageIndex>& matrix,
                         Func&& func, Args&&... args) {

        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        using SparseMatrixType = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

        // Iterate over non-zero elements
        for (int k = 0; k < matrix.outerSize(); ++k) {
            for (typename SparseMatrixType::InnerIterator it(matrix, k); it; ++it) {
                // Apply the function to the value
                it.valueRef() = func(it.value(), std::forward<Args>(args)...);
            }
        }
    }




/**
 * @brief Applies a function element-wise to a Dense matrix (similar to MATLAB's arrayfun).
 *
 * @tparam Derived The type of the input matrix (e.g., MatrixXd).
 * @tparam Func The type of the callable function.
 * @tparam Args Variadic types for additional arguments.
 * @param matrix Input dense matrix.
 * @param func The function to apply: Scalar func(Scalar, Args...).
 * @param args Additional arguments to forward to the function.
 * @return Derived The resulting matrix with the same type as input.
 */
    template <typename Derived, typename Func, typename... Args>
    Derived arrayfun(const Eigen::MatrixBase<Derived>& matrix, Func&& func, Args&&... args) {
        // Create a copy of the matrix
        Derived result = matrix.derived();

        // Call the in-place version on the copy
        arrayfunInplace(result, std::forward<Func>(func), std::forward<Args>(args)...);

        return result;
    }

/**
 * @brief Applies a function element-wise to a Sparse matrix.
 *
 * @tparam Scalar The scalar type.
 * @tparam Options The storage options (ColMajor, RowMajor).
 * @tparam StorageIndex The index type.
 * @tparam Func The type of the callable function.
 * @tparam Args Variadic types for additional arguments.
 * @param matrix Input sparse matrix.
 * @param func The function to apply: Scalar func(Scalar, Args...).
 * @param args Additional arguments to forward to the function.
 * @return Eigen::SparseMatrix<Scalar, Options, StorageIndex> The resulting sparse matrix.
 */
    template <typename Scalar, int Options, typename StorageIndex, typename Func, typename... Args>
    Eigen::SparseMatrix<Scalar, Options, StorageIndex> arrayfun(
            const Eigen::SparseMatrix<Scalar, Options, StorageIndex>& matrix,
            Func&& func, Args&&... args) {

        // Create a copy of the matrix
        Eigen::SparseMatrix<Scalar, Options, StorageIndex> result = matrix;

        // Call the in-place version on the copy
        arrayfunInplace(result, std::forward<Func>(func), std::forward<Args>(args)...);

        return result;
    }

} // namespace gsp::matrix


#endif // LIBGSP_MATRIX_H
