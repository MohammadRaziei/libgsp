//
// Created by Mohammad on 7/24/2025
//

#ifndef LIBGSP_MATRIX_H
#define LIBGSP_MATRIX_H
#pragma once

#include <cstdint>
#include <Eigen/Dense>
#include <Eigen/SparseCore>
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


} // namespace gsp::matrix


#endif // LIBGSP_MATRIX_H
