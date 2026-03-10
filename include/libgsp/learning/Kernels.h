//
// Created by mohammad on 3/10/26.
//

#ifndef LIBGSP_KERNELS_H
#define LIBGSP_KERNELS_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>

namespace gsp {

/**
 * @brief Applies a function element-wise to a Dense matrix (similar to MATLAB's arrayfun).
 *
 * @tparam Derived The type of the input matrix (e.g., MatrixXd).
 * @tparam Func The type of the callable function (lambda, functor, etc.).
 * @tparam Args Variadic types for additional arguments to pass to the function.
 * @param matrix Input dense matrix.
 * @param func The function to apply: Scalar func(Scalar, Args...).
 * @param args Additional arguments to forward to the function.
 * @return Derived The resulting matrix with the same type as input.
 */
    template <typename Derived, typename Func, typename... Args>
    Derived arrayfun(const Eigen::MatrixBase<Derived>& matrix, Func&& func, Args&&... args) {
        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<typename Derived::Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        // Create a copy of the matrix to store results
        Derived result = matrix.derived();

        // Efficiently iterate over the data using Eigen's raw data access
        // This is faster than using operator() for large dense matrices.
        auto* data_ptr = result.data();
        const auto size = result.size();

        for (Eigen::Index i = 0; i < size; ++i) {
            data_ptr[i] = func(data_ptr[i], std::forward<Args>(args)...);
        }

        return result;
    }

/**
 * @brief Applies a function element-wise to a Sparse matrix.
 *
 * This function iterates only over non-zero elements, preserving sparsity.
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
            Func&& func,
            Args&&... args) {

        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        using SparseMatrixType = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

        // Create a copy to modify
        SparseMatrixType result = matrix;

        // Iterate over non-zero elements
        for (int k = 0; k < result.outerSize(); ++k) {
            for (typename SparseMatrixType::InnerIterator it(result, k); it; ++it) {
                // Apply the function to the value
                it.valueRef() = func(it.value(), std::forward<Args>(args)...);
            }
        }

        return result;
    }

// Helper Lambdas
    namespace detail {
        // Gaussian: exp(-x^2 / (2 * sigma^2))
        auto gaussian_lambda = [](auto val, auto sigma2) {
            return std::exp(-(val * val) / (2.0 * sigma2));
        };

        // Exponential: exp(-x / sigma)
        auto exponential_lambda = [](auto val, auto sigma) {
            return std::exp(-val / sigma);
        };


        template <typename Derived, typename Func, typename... Args>
        Derived arrayfunKernel(
                const Eigen::MatrixBase<Derived>& distance,
                double thresh, Func&& func, Args&&... args) {
            auto weights = arrayfun(distance.derived(), func, std::forward<Args>(args)...);

            if (thresh > 0.0) {
                weights = (weights.array() >= thresh).select(weights, static_cast<typename std::decay_t<Derived>::Scalar>(0));
            }
            return weights;
        }

        template <typename Scalar, int Options, typename StorageIndex, typename Func, typename... Args>
        Eigen::SparseMatrix<Scalar, Options, StorageIndex> arrayfunKernel(
                const Eigen::SparseMatrix<Scalar, Options, StorageIndex>& distance,
                double thresh, Func&& func, Args&&... args) {

            auto weights = arrayfun(distance, func, std::forward<Args>(args)...);

            if (thresh > 0.0) {
                weights.prune(thresh);
            }
            return weights;
        }

    }


// --- Gaussian Kernel ---

    template <class Matrix>
    Matrix gaussianKernel(const Matrix& distance, double sigma2, double thresh = 1e-6) {
        return detail::arrayfunKernel(distance, thresh, detail::gaussian_lambda, sigma2);
    }
// --- Exponential Kernel ---

    template <class Matrix>
    Matrix exponentialKernel(const Matrix& distance, double sigma, double thresh = 1e-6) {
        return detail::arrayfunKernel(distance, thresh, detail::exponential_lambda, sigma);
    }

//    eigen cauchyKernel(eigen distance, double sigma, double thresh);
//    eigen inverseMultiquadricKernel(eigen distance, double sigma, double thresh);

}

#endif //LIBGSP_KERNELS_H
