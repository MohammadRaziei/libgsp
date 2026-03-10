//
// Created by mohammad on 3/10/26.
//

#ifndef LIBGSP_KERNELS_H
#define LIBGSP_KERNELS_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>
#include "libgsp/utils/Matrix.h"

namespace gsp {
    namespace detail {

        /**
 * @brief Applies a function and threshold to a Dense matrix in-place.
 *
 * @tparam Derived The type of the input matrix.
 * @tparam Func The type of the callable function.
 * @tparam Args Variadic types for additional arguments.
 * @param distance Input distance matrix (modified in-place).
 * @param thresh Threshold to zero-out small weights.
 * @param func The function to apply.
 * @param args Additional arguments to forward to the function.
 */
        template <typename Derived, typename Func, typename... Args>
        void arrayfunKernelInplace(Eigen::MatrixBase<Derived>& distance,
                                   double thresh,
                                   Func&& func,
                                   Args&&... args) {

            // Apply the function in-place
            gsp::matrix::arrayfunInplace(distance, std::forward<Func>(func), std::forward<Args>(args)...);

            // Apply thresholding
            if (thresh > 0.0) {
                // We use array() for coefficient-wise operations
                distance.derived() = (distance.derived().array() >= thresh)
                        .select(distance.derived(), static_cast<typename std::decay_t<Derived>::Scalar>(0));
            }
        }

/**
 * @brief Applies a function and threshold to a Sparse matrix in-place.
 *
 * @tparam Scalar The scalar type.
 * @tparam Options The storage options.
 * @tparam StorageIndex The index type.
 * @tparam Func The type of the callable function.
 * @tparam Args Variadic types for additional arguments.
 * @param distance Input distance matrix (modified in-place).
 * @param thresh Threshold to zero-out small weights.
 * @param func The function to apply.
 * @param args Additional arguments to forward to the function.
 */
        template <typename Scalar, int Options, typename StorageIndex, typename Func, typename... Args>
        void arrayfunKernelInplace(Eigen::SparseMatrix<Scalar, Options, StorageIndex>& distance,
                                   double thresh,
                                   Func&& func,
                                   Args&&... args) {

            // Apply the function in-place
            gsp::matrix::arrayfunInplace(distance, std::forward<Func>(func), std::forward<Args>(args)...);

            // Apply thresholding
            if (thresh > 0.0) {
                distance.prune(thresh);
            }
        }

    }







    // ==================================================================================
    // Gaussian Kernel
    // ==================================================================================

    /**
     * @brief Computes the Gaussian kernel weights from a distance matrix (In-place).
     *
     * This function modifies the input matrix directly.
     * Formula: $w_{ij} = \exp\left(-\frac{d_{ij}^2}{2\sigma^2}\right)$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sigma2 The variance parameter ($\sigma^2$) of the Gaussian distribution.
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void gaussianKernelInplace(Matrix& distance, double sigma2, double thresh = 1e-6) {
        auto gaussian_lambda = [sigma2](typename std::decay_t<Matrix>::Scalar& val)  {
            val = std::exp(-(val * val) / (2.0 * sigma2));
        };
        detail::arrayfunKernelInplace(distance, thresh, gaussian_lambda);
    }

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
        Matrix result = distance; // Create a copy
        gaussianKernelInplace(result, sigma2, thresh);
        return result;
    }

    // ==================================================================================
    // Exponential Kernel
    // ==================================================================================

    /**
     * @brief Computes the Exponential kernel weights from a distance matrix (In-place).
     *
     * This function modifies the input matrix directly.
     * Formula: $w_{ij} = \exp\left(-\frac{d_{ij}}{\sigma}\right)$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sigma The decay parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void exponentialKernelInplace(Matrix& distance, double sigma, double thresh = 1e-6) {
        auto exponential_lambda = [sigma](typename std::decay_t<Matrix>::Scalar& val)  {
            val = std::exp(-val / sigma);
        };
        detail::arrayfunKernelInplace(distance, thresh, exponential_lambda);
    }

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
        Matrix result = distance;
        exponentialKernelInplace(result, sigma, thresh);
        return result;
    }

    // ==================================================================================
    // Cauchy Kernel
    // ==================================================================================

    /**
     * @brief Computes the Cauchy kernel weights from a distance matrix (In-place).
     *
     * This function modifies the input matrix directly.
     * Formula: $w_{ij} = \frac{1}{1 + \left(\frac{d_{ij}}{\sigma}\right)^2}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sigma The scale parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void cauchyKernelInplace(Matrix& distance, double sigma, double thresh = 1e-6) {
        auto cauchy_lambda = [sigma](typename std::decay_t<Matrix>::Scalar& val)  {
            auto ratio = val / sigma;
            val = 1.0 / (1.0 + ratio * ratio);
        };
        detail::arrayfunKernelInplace(distance, thresh, cauchy_lambda);
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
        Matrix result = distance;
        cauchyKernelInplace(result, sigma, thresh);
        return result;
    }

    // ==================================================================================
    // Inverse Multiquadric Kernel
    // ==================================================================================

    /**
     * @brief Computes the Inverse Multiquadric kernel weights from a distance matrix (In-place).
     *
     * This function modifies the input matrix directly.
     * Formula: $w_{ij} = \frac{1}{\sqrt{d_{ij}^2 + \sigma^2}}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sigma The smoothness parameter ($\sigma$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void inverseMultiquadricKernelInplace(Matrix& distance, double sigma, double thresh = 1e-6) {
        auto inverse_multiquadric_lambda = [sigma](typename std::decay_t<Matrix>::Scalar& val)  {
            val = 1.0 / std::sqrt((val * val) + (sigma * sigma));
        };
        detail::arrayfunKernelInplace(distance, thresh, inverse_multiquadric_lambda);
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
        Matrix result = distance;
        inverseMultiquadricKernelInplace(result, sigma, thresh);
        return result;
    }

    // ==================================================================================
    // Rational Quadratic Kernel
    // ==================================================================================

    /**
     * @brief Computes the Rational Quadratic kernel weights from a distance matrix (In-place).
     *
     * This function modifies the input matrix directly.
     * It can be interpreted as an infinite sum of Gaussian kernels with different length scales.
     * Formula: $w_{ij} = 1 - \frac{d_{ij}^2}{d_{ij}^2 + \sigma^2}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sigma2 The scale parameter ($\sigma^2$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void rationalQuadraticKernelInplace(Matrix& distance, double sigma2, double thresh = 1e-6) {
        auto rational_quadratic_lambda = [sigma2](typename std::decay_t<Matrix>::Scalar& val)  {
            auto val_sq = val * val;
            val = 1.0 - (val_sq / (val_sq + sigma2));
        };
        detail::arrayfunKernelInplace(distance, thresh, rational_quadratic_lambda);
    }

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
        Matrix result = distance;
        rationalQuadraticKernelInplace(result, sigma, thresh);
        return result;
    }

    // ==================================================================================
    // Epsilon Neighborhood
    // ==================================================================================

    /**
     * @brief Computes the Epsilon Neighborhood graph weights (In-place).
     *
     * This function modifies the input matrix directly.
     * It creates a binary weight matrix where an edge exists if the distance is less than epsilon.
     * Formula: $w_{ij} = \begin{cases} 1 & \text{if } d_{ij} < \epsilon \\ 0 & \text{otherwise} \end{cases}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param epsilon The radius threshold ($\epsilon$).
     * @param thresh Threshold to zero-out small weights (default: 1e-6). Note: Output is already 0 or 1.
     */
    template <class Matrix>
    void epsilonNeighborhoodKernelInplace(Matrix& distance, double epsilon, double thresh = 1e-6) {
        auto epsilon_lambda = [epsilon](typename std::decay_t<Matrix>::Scalar& val)  {
            val = (val < epsilon) ? 1.0 : 0.0;
        };
        detail::arrayfunKernelInplace(distance, thresh, epsilon_lambda);
    }

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
    Matrix epsilonNeighborhoodKernel(const Matrix& distance, double epsilon, double thresh = 1e-6) {
        Matrix result = distance;
        epsilonNeighborhoodKernelInplace(result, epsilon, thresh);
        return result;
    }

    // ==================================================================================
    // from Sup (Sup-normalized)
    // ==================================================================================

    /**
     * @brief Computes weights based on the maximum distance (Sup-normalized) (In-place).
     *
     * This function modifies the input matrix directly.
     * It transforms distances to similarities by subtracting them from the
     * maximum distance value (Sup). It results in a linear decay of weights.
     * Formula: $w_{ij} = \sup(D) - d_{ij}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param sup The maximum distance value. If set to 0, it is automatically computed as max(distance).
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void fromSupKernelInplace(Matrix& distance, double sup, double thresh = 1e-6) {
        if (sup == 0.0) sup = gsp::matrix::maxCoeff(distance);
        auto sup_minus_lambda = [sup](typename std::decay_t<Matrix>::Scalar& val) {
            val = sup - val;
        };
        detail::arrayfunKernelInplace(distance, thresh, sup_minus_lambda);
    }

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
    Matrix fromSupKernel(const Matrix& distance, double sup, double thresh = 1e-6) {
        Matrix result = distance;
        fromSupKernelInplace(result, sup, thresh);
        return result;
    }

    // ==================================================================================
    // Inverse Dist
    // ==================================================================================

    /**
     * @brief Computes the Inverse Distance weights (In-place).
     *
     * This function modifies the input matrix directly.
     * It computes the inverse of the distance, adding a small epsilon
     * to avoid division by zero.
     * Formula: $w_{ij} = \frac{1}{d_{ij} + \epsilon}$
     *
     * @tparam Matrix The type of the distance matrix (Dense or Sparse).
     * @param distance Input distance matrix (modified in-place).
     * @param eps A small constant added to the distance to prevent division by zero.
     * @param thresh Threshold to zero-out small weights (default: 1e-6).
     */
    template <class Matrix>
    void inverseDistKernelInplace(Matrix& distance, double eps, double thresh = 1e-6) {
        auto inverse_lambda = [eps](typename std::decay_t<Matrix>::Scalar& val)  {
            val = 1.0 / (val + eps);
        };
        detail::arrayfunKernelInplace(distance, thresh, inverse_lambda);
    }

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
    Matrix inverseDistKernel(const Matrix& distance, double eps, double thresh = 1e-6) {
        Matrix result = distance;
        inverseDistKernelInplace(result, eps, thresh);
        return result;
    }





}

#endif //LIBGSP_KERNELS_H
