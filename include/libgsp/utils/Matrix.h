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
Eigen::SparseMatrix<Scalar, Options, Index> kronSum(const Eigen::SparseMatrix<Scalar, Options, Index>& a,
                                                    const Eigen::SparseMatrix<Scalar, Options, Index>& b) {
    const Eigen::MatrixX<Scalar> Ia = Eigen::MatrixX<Scalar>::Identity(a.rows(), a.rows());
    const Eigen::MatrixX<Scalar> Ib = Eigen::MatrixX<Scalar>::Identity(b.rows(), b.rows());
    return Eigen::KroneckerProductSparse(a, Ib) * Eigen::KroneckerProductSparse(Ia, b);
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC> kronSum(const Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& a,
                                                           const Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& b) {
    const Eigen::MatrixX<Scalar> Ia = Eigen::MatrixX<Scalar>::Identity(a.rows(), a.rows());
    const Eigen::MatrixX<Scalar> Ib = Eigen::MatrixX<Scalar>::Identity(b.rows(), b.rows());
    return Eigen::KroneckerProduct(a, Ib) * Eigen::KroneckerProduct(Ia, b);
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
