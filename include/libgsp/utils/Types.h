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

template <typename Scalar>
using densematrix_t = Eigen::MatrixX<Scalar>;
template <typename Scalar>
using sparsematrix_t = Eigen::SparseMatrix<Scalar, Eigen::RowMajor>;

using densematrix = densematrix_t<double>;
using sparsematrix = sparsematrix_t<double>;

template <typename Scalar>
using densevector_t = typename Eigen::Matrix<Scalar, Eigen::Dynamic, 1, Eigen::ColMajor>;

using half_t = Eigen::half;
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
    static const int rows = R, cols = C;
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


template <typename T>
using float_of =
    std::conditional_t<
        (sizeof(T) <= sizeof(half_t)), half_t,
        std::conditional_t<
            (sizeof(T) <= sizeof(float)), float,
            std::conditional_t<(sizeof(T) <= sizeof(double)), double, long double>>>;


template <typename T>
struct is_floating_point
    : std::bool_constant<
          std::is_floating_point_v<T> ||
          std::is_same_v<std::remove_cv_t<T>, Eigen::half>
      > {};

template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;




template <class Matrix>
using matrix_float_t =
    std::conditional_t<
            is_eigen_sparse<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value,
            Eigen::SparseMatrix<
                    float_of<elem_t<Matrix>>,
                    std::remove_cv_t<std::remove_reference_t<Matrix>>::Options,
                    typename std::remove_cv_t<std::remove_reference_t<Matrix>>::StorageIndex
            >,
            std::conditional_t<
                    is_eigen_dense<std::remove_cv_t<std::remove_reference_t<Matrix>>>::value,
                    Eigen::Matrix<
                            float_of<elem_t<Matrix>>,
                            std::remove_cv_t<std::remove_reference_t<Matrix>>::RowsAtCompileTime,
                            std::remove_cv_t<std::remove_reference_t<Matrix>>::ColsAtCompileTime,
                            std::remove_cv_t<std::remove_reference_t<Matrix>>::Options,
                            std::remove_cv_t<std::remove_reference_t<Matrix>>::MaxRowsAtCompileTime,
                            std::remove_cv_t<std::remove_reference_t<Matrix>>::MaxColsAtCompileTime
                    >,
                    void
            >
    >;














} // namespace gsp::types










#endif  // LIBGSP_TYPES_H
