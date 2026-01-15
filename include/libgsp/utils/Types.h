//
// gsp::types — Eigen-based type traits (minimal, zero runtime cost).
//

#ifndef LIBGSP_TYPES_H
#define LIBGSP_TYPES_H
#pragma once

#include <type_traits>
#include <Eigen/Core>
#include <Eigen/SparseCore>

namespace gsp {
using densematrix = Eigen::MatrixXd;
using sparsematrix = Eigen::SparseMatrix<double, Eigen::RowMajor>;

template <typename Scalar>
using densevector = typename Eigen::Matrix<Scalar, Eigen::Dynamic, 1, Eigen::ColMajor>;
}

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


// ---- type traits: detect Eigen sparse vs dense ----
template <typename T> struct is_eigen_sparse : std::false_type {
    using scalar = void;
    static const int options = -1;
};
template <typename S, int O, typename I>
struct is_eigen_sparse<Eigen::SparseMatrix<S, O, I>> : std::true_type {
    using scalar = S;
    static const int options = O;
    using storage_index = I;
};

template <typename T> struct is_eigen_dense : std::false_type {
    using scalar = void;
    static const int options = -1;
};
template <typename S, int R, int C, int O, int MR, int MC>
struct is_eigen_dense<Eigen::Matrix<S, R, C, O, MR, MC>> : std::true_type {
    using scalar = S;
    static const int options = O;
    static const int max_rows = MR;
    static const int max_cols = MC;
};


// Any Eigen::Matrix (includes vectors)
template <typename S, int R, int C, int O, int MR, int MC>
struct is_eigenmatrix<Eigen::Matrix<S, R, C, O, MR, MC>> : is_eigen_dense<Eigen::Matrix<S, R, C, O, MR, MC>> {};

// Sparse matrix
template <typename S, int O, typename I>
struct is_eigenmatrix<Eigen::SparseMatrix<S, O, I>> : is_eigen_sparse<Eigen::SparseMatrix<S, O, I>> {};

// Dense column vectors (Nx1) and row vectors (1xM)
template <typename S, int R, int O, int MR, int MC>
struct is_eigenvector<Eigen::Matrix<S, R, 1, O, MR, MC>> : is_eigen_dense<Eigen::Matrix<S, R, 1, O, MR, MC>> {};
template <typename S, int C, int O, int MR, int MC>
struct is_eigenvector<Eigen::Matrix<S, 1, C, O, MR, MC>> : is_eigen_dense<Eigen::Matrix<S, 1, C, O, MR, MC>> {};

// Sparse vector
template <typename S, int O, typename I>
struct is_eigenvector<Eigen::SparseVector<S, O, I>> :  is_eigen_sparse<Eigen::SparseMatrix<S, O, I>> {};

// ----- optional aliases to keep old call sites working -----
template <typename T> using is_matrix = is_eigenmatrix<T>;
template <typename T> using is_vector = is_eigenvector<T>;


template<typename Matrix> struct vector_of { using type = void;};

// Dense matrix → VectorXd / VectorXcd ...
template<typename Scalar, int Rows, int Cols, int Options, int MR, int MC>
struct vector_of<Eigen::Matrix<Scalar, Rows, Cols, Options, MR, MC>> {
    using type = Eigen::Matrix<Scalar, Rows, 1>;
};

// Sparse matrix → SparseVector<double> / SparseVector<complex> ...
template<typename Scalar, int Options, typename Index>
struct vector_of<Eigen::SparseMatrix<Scalar, Options, Index>> {
    using type = Eigen::SparseVector<Scalar, Options, Index>;
};

template<typename Matrix>
using vector_t = typename vector_of<std::remove_cv_t<std::remove_reference_t<Matrix>>>::type;



template <typename Matrix> using densevector_m = typename Eigen::Matrix<elem_t<Matrix>, Eigen::Dynamic, 1, Eigen::ColMajor>;


} // namespace gsp::types

#endif  // LIBGSP_TYPES_H
