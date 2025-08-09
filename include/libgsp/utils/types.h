//
// gsp::types — Eigen-based type traits (minimal, zero runtime cost).
//

#ifndef LIBGSP_TYPES_H
#define LIBGSP_TYPES_H
#pragma once

#include <type_traits>
#include <Eigen/Core>
#include <Eigen/SparseCore>

namespace gsp::types {

// ----- element type -----
template <typename T>
struct typeofelement { using type = void; };

template <typename S, int R, int C, int O, int MR, int MC>
struct typeofelement<Eigen::Matrix<S, R, C, O, MR, MC>> { using type = S; };

template <typename S, int O, typename I>
struct typeofelement<Eigen::SparseMatrix<S, O, I>> { using type = S; };

template <typename S, int O, typename I>
struct typeofelement<Eigen::SparseVector<S, O, I>> { using type = S; };

template <typename Matrix>
using elem_t = typename typeofelement<std::remove_cv_t<std::remove_reference_t<Matrix>>>::type;

// ----- matrix / vector detection -----
template <typename T> struct is_eigenmatrix : std::false_type {};
template <typename T> struct is_eigenvector : std::false_type {};

// Any Eigen::Matrix (includes vectors)
template <typename S, int R, int C, int O, int MR, int MC>
struct is_eigenmatrix<Eigen::Matrix<S, R, C, O, MR, MC>> : std::true_type {};

// Sparse matrix
template <typename S, int O, typename I>
struct is_eigenmatrix<Eigen::SparseMatrix<S, O, I>> : std::true_type {};

// Dense column vectors (Nx1) and row vectors (1xM)
template <typename S, int R, int O, int MR, int MC>
struct is_eigenvector<Eigen::Matrix<S, R, 1, O, MR, MC>> : std::true_type {};
template <typename S, int C, int O, int MR, int MC>
struct is_eigenvector<Eigen::Matrix<S, 1, C, O, MR, MC>> : std::true_type {};

// Sparse vector
template <typename S, int O, typename I>
struct is_eigenvector<Eigen::SparseVector<S, O, I>> : std::true_type {};

// ----- optional aliases to keep old call sites working -----
template <typename T> using is_matrix = is_eigenmatrix<T>;
template <typename T> using is_vector = is_eigenvector<T>;

} // namespace gsp::types

#endif  // LIBGSP_TYPES_H
