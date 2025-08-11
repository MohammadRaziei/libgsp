//
// Created by Mohammad on 7/24/2025
//

#ifndef LIBGSP_MATRIX_H
#define LIBGSP_MATRIX_H
#pragma once

#include <cstdint>
#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include "libgsp/utils/types.h"

namespace gsp::matrix {
// ======== Declarations ========

// allocate
template<typename Matrix>
void allocate(Matrix& matrix, uint32_t rows, uint32_t cols);

// fillZero
template<typename Matrix>
void fillZero(Matrix& matrix);

// free
template<typename Matrix>
void free(Matrix& matrix);

// setElement
template<typename Matrix>
void setElement(Matrix& matrix,
                uint32_t row, uint32_t col,
                gsp::types::elem_t<Matrix> el);

// getElement
template<typename Matrix>
gsp::types::elem_t<Matrix>
getElement(Matrix& matrix, uint32_t row, uint32_t col);

} // namespace gsp::matrix

// ======== Implementations ========

// ---------- allocate ----------
template<typename Scalar, int Options, typename Index>
void gsp::matrix::allocate(Eigen::SparseMatrix<Scalar, Options, Index>& m,
              uint32_t rows, uint32_t cols) {
    m.resize(static_cast<Eigen::Index>(rows),
             static_cast<Eigen::Index>(cols));
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void gsp::matrix::allocate(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
              uint32_t rows, uint32_t cols) {
    m.resize(static_cast<Eigen::Index>(rows),
             static_cast<Eigen::Index>(cols));
}

// ---------- fillZero ----------
template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void gsp::matrix::fillZero(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& matrix) {
    matrix.setZero();
}

template<typename Scalar, int Options, typename Index>
void gsp::matrix::fillZero(Eigen::SparseMatrix<Scalar, Options, Index>& matrix) {
    matrix.setZero();
}

// ---------- free ----------
template<typename Scalar, int Options, typename Index>
void gsp::matrix::free(Eigen::SparseMatrix<Scalar, Options, Index>& m) {
    m.resize(0, 0);
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void gsp::matrix::free(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m) {
    m.resize(0, 0);
}

// ---------- setElement ----------
template<typename Scalar, int Options, typename Index>
void gsp::matrix::setElement(Eigen::SparseMatrix<Scalar, Options, Index>& m,
                uint32_t r, uint32_t c, Scalar el) {
    if (m.isCompressed()) m.uncompress();
    m.coeffRef(static_cast<Eigen::Index>(r),
               static_cast<Eigen::Index>(c)) = el;
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
void gsp::matrix::setElement(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
                uint32_t r, uint32_t c, Scalar el) {
    m(static_cast<Eigen::Index>(r),
      static_cast<Eigen::Index>(c)) = el;
}

// ---------- getElement ----------
template<typename Scalar, int Options, typename Index>
Scalar gsp::matrix::getElement(Eigen::SparseMatrix<Scalar, Options, Index>& m,
                  uint32_t r, uint32_t c) {
    return m.coeff(static_cast<Eigen::Index>(r),
                   static_cast<Eigen::Index>(c));
}

template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
Scalar gsp::matrix::getElement(Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>& m,
                  uint32_t r, uint32_t c) {
    return m(static_cast<Eigen::Index>(r),
             static_cast<Eigen::Index>(c));
}


#endif // LIBGSP_MATRIX_H
