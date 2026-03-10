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
    /**
     * @brief Computes the Gaussian kernel weights from a distance matrix.
     *
     * This function applies the Gaussian (RBF) kernel element-wise to the distance matrix.
     * Formula: $w_{ij} = \exp\left(-\frac{d_{ij}^2}{2\sigma^2}\right)$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sigma2 The variance parameter ($\sigma^2$) of the Gaussian distribution.
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix gaussianKernel(const Matrix& distance, double sigma2, double thresh = 1e-6) {
        // Gaussian: exp(-x^2 / (2 * sigma^2))
        auto gaussian_lambda = [](auto val, auto sigma2) {
            return std::exp(-(val * val) / (2.0 * sigma2));
        };
        return detail::arrayfunKernel(distance, thresh, gaussian_lambda, sigma2);
    }

    // --- Exponential Kernel ---
    /**
     * @brief Computes the Exponential kernel weights from a distance matrix.
     *
     * This function applies the Exponential kernel element-wise to the distance matrix.
     * Formula: $w_{ij} = \exp\left(-\frac{d_{ij}}{\sigma}\right)$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sigma The decay parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix exponentialKernel(const Matrix& distance, double sigma, double thresh = 1e-6) {
        // Exponential: exp(-x / sigma)
        auto exponential_lambda = [](auto val, auto sigma) {
            return std::exp(-val / sigma);
        };
        return detail::arrayfunKernel(distance, thresh, exponential_lambda, sigma);
    }

    /**
     * @brief Computes the Cauchy kernel weights from a distance matrix.
     *
     * This function applies the Cauchy kernel element-wise to the distance matrix.
     * Formula: $w_{ij} = \frac{1}{1 + \left(\frac{d_{ij}}{\sigma}\right)^2}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sigma The scale parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix cauchyKernel(const Matrix& distance, double sigma, double thresh = 1e-6) {
        // Formula: 1 / (1 + (val / sigma)^2)
        auto cauchy_lambda = [](auto val, auto sigma) {
            auto ratio = val / sigma;
            return 1.0 / (1.0 + ratio * ratio);
        };
        return detail::arrayfunKernel(distance, thresh, cauchy_lambda, sigma);
    }

    /**
     * @brief Computes the Inverse Multiquadric kernel weights from a distance matrix.
     *
     * This function applies the Inverse Multiquadric kernel element-wise to the distance matrix.
     * Formula: $w_{ij} = \frac{1}{\sqrt{d_{ij}^2 + \sigma^2}}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sigma The smoothness parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix inverseMultiquadricKernel(const Matrix& distance, double sigma, double thresh = 1e-6) {
        // Formula: 1 / sqrt(val^2 + sigma^2)
        auto inverse_multiquadric_lambda = [](auto val, auto sigma) {
            return 1.0 / std::sqrt((val * val) + (sigma * sigma));
        };
        return detail::arrayfunKernel(distance, thresh, inverse_multiquadric_lambda, sigma);
    }

    // --- Rational Quadratic Kernel ---
    /**
     * @brief Computes the Rational Quadratic kernel weights from a distance matrix.
     *
     * This function applies the Rational Quadratic kernel element-wise to the distance matrix.
     * It can be interpreted as an infinite sum of Gaussian kernels with different length scales.
     * Formula: $w_{ij} = 1 - \frac{d_{ij}^2}{d_{ij}^2 + \sigma^2}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sigma The scale parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix rationalQuadraticKernel(const Matrix& distance, double sigma, double thresh = 1e-6) {
        // Formula: 1 - (d^2 / (d^2 + sigma^2))
        auto rational_quadratic_lambda = [](auto val, auto sigma) {
            auto val_sq = val * val;
            auto sigma_sq = sigma * sigma;
            return 1.0 - (val_sq / (val_sq + sigma_sq));
        };
        return detail::arrayfunKernel(distance, thresh, rational_quadratic_lambda, sigma);
    }

    // --- Epsilon Neighborhood ---
    /**
     * @brief Computes the Epsilon Neighborhood graph weights.
     *
     * This function creates a binary weight matrix where an edge exists if the distance
     * is less than epsilon.
     * Formula: $w_{ij} = \begin{cases} 1 & \text{if } d_{ij} < \epsilon \\ 0 & \text{otherwise} \end{cases}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param epsilon The radius threshold ($\epsilon$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6). Note: Output is already 0 or 1.
     * @return Matrix The resulting binary weight matrix.
     */
    template <class Matrix>
    Matrix epsilonNeighborhood(const Matrix& distance, double epsilon, double thresh = 1e-6) {
        // Formula: 1 if d < epsilon else 0
        // Note: thresh is usually not needed here as output is already 0 or 1,
        // but kept for interface consistency.
        auto epsilon_lambda = [](auto val, auto epsilon) {
            return (val < epsilon) ? 1.0 : 0.0;
        };
        return detail::arrayfunKernel(distance, thresh, epsilon_lambda, epsilon);
    }

    // --- from Sup ---
    /**
     * @brief Computes weights based on the maximum distance (Sup-normalized).
     *
     * This function transforms distances to similarities by subtracting them from the
     * maximum distance value (Sup). It results in a linear decay of weights.
     * Formula: $w_{ij} = \sup(D) - d_{ij}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param sup The maximum distance value. If set to 0, it is automatically computed as max(distance).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix fromSup(const Matrix& distance, double sup, double thresh = 1e-6) {
        // Determine the sup value
        if (sup == 0.0) sup = distance.maxCoeff();
        // Formula: sup - d
        auto sup_minus_lambda = [](auto val, auto sup) {
            return sup - val;
        };
        return detail::arrayfunKernel(distance, thresh, sup_minus_lambda, sup);
    }

    // --- Inverse Dist ---
    /**
     * @brief Computes the Inverse Distance weights.
     *
     * This function computes the inverse of the distance, adding a small epsilon
     * to avoid division by zero.
     * Formula: $w_{ij} = \frac{1}{d_{ij} + \epsilon}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix.
     * @param eps A small constant added to the distance to prevent division by zero.
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     * @return Matrix The resulting weight matrix.
     */
    template <class Matrix>
    Matrix inverseDist(const Matrix& distance, double eps, double thresh = 1e-6) {
        // Formula: 1 / (d + eps)
        auto inverse_lambda = [](auto val, auto eps) {
            return 1.0 / (val + eps);
        };
        return detail::arrayfunKernel(distance, thresh, inverse_lambda, eps);
    }
}

#endif //LIBGSP_KERNELS_H
