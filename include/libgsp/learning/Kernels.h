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
 * @brief Computes the Gaussian kernel for Dense matrices.
 *
 * @param distance Input dense distance matrix.
 * @param sigma2 The variance parameter (sigma squared).
 * @param thresh Threshold to zero-out small weights.
 * @return Eigen::Matrix<Scalar, Rows, Cols> The resulting dense weight matrix.
 */
template <typename Scalar, int Rows, int Cols,  int Options, int MaxRows, int MaxCols>
Eigen::Matrix<Scalar, Rows, Cols> gaussianKernel(
            const Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>& distance,
            double sigma2, double thresh = 1e-6) {
    // Static assertion to ensure the scalar type is floating point
    static_assert(std::is_floating_point<Scalar>::value,
                  "Scalar type must be floating point (float or double).");

    // Compute Gaussian kernel: exp(-d^2 / (2 * sigma^2))
    // Using Eigen's expression templates for optimal performance.
    Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> weights =
            (-distance.array().square() / (2 * sigma2)).exp();

    // Apply thresholding
    if (thresh > 0.0) {
        weights = (weights.array() >= thresh).select(weights, 0.0);
    }

    return weights;
}


/**
 * @brief Computes the Gaussian kernel for Sparse matrices without full dense conversion.
 *
 * This function is memory efficient and avoids converting the entire matrix to dense.
 * It uses sparse-sparse operations and direct value iteration for the exponential.
 *
 * @tparam Scalar The scalar type (e.g., float, double).
 * @tparam Options The storage options (e.g., ColMajor, RowMajor).
 * @tparam StorageIndex The index type (e.g., int).
 * @param distance Input sparse distance matrix.
 * @param sigma2 The variance parameter (sigma squared).
 * @param thresh Threshold to zero-out small weights.
 * @return Eigen::SparseMatrix<Scalar, Options, StorageIndex> The resulting sparse weight matrix.
 */
template <typename Scalar, int Options, typename StorageIndex>
Eigen::SparseMatrix<Scalar, Options, StorageIndex> gaussianKernel(
        const Eigen::SparseMatrix<Scalar, Options, StorageIndex>& distance,
        double sigma2,
        double thresh = 1e-6) {

    // Static assertion to ensure the scalar type is floating point
    static_assert(std::is_floating_point<Scalar>::value,
                  "Scalar type must be floating point (float or double).");

    using SparseMatrixType = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

    // 1. Compute Squared Distances: D_sq = D .* D (Hadamard product)
    // This preserves sparsity perfectly.
    SparseMatrixType distSquared = distance.cwiseProduct(distance);

    // 2. Scale by Gaussian factor: Scaled = -1 / (2 * sigma^2) * D_sq
    // We perform a sparse-sparse multiplication with a diagonal-like matrix
    // or simply iterate and scale. Iteration is O(NNZ) and very cache-friendly.
    const Scalar scale_factor = Scalar(-1.0 / (2 * sigma2));

    // We create a copy to store the scaled values before exp()
    SparseMatrixType scaledMatrix = distSquared;

    // Iterate over non-zero elements to apply scaling and exponential
    // This is the most memory-efficient way for the exp() operation on sparse data.
    for (int k = 0; k < scaledMatrix.outerSize(); ++k) {
        for (typename SparseMatrixType::InnerIterator it(scaledMatrix, k); it; ++it) {
            Scalar val = it.value();
            // Apply scaling: val * (-1 / 2sigma^2)
            val *= scale_factor;

            // Apply Exponential: exp(val)
            // Note: exp(0) = 1, so zeros remain zeros (sparsity preserved for 0-distances)
            // However, if distance was 0, exp(0)=1, which creates a new non-zero.
            // This is unavoidable for the diagonal of the graph.
            it.valueRef() = std::exp(val);
        }
    }

    // 3. Apply Thresholding
    if (thresh > 0.0) {
        // prune() removes elements smaller than the reference (thresh)
        // It also removes elements that are exactly zero.
        scaledMatrix.prune(thresh);
    }

    return scaledMatrix;
}


/**
 * @brief Computes the Exponential kernel for Dense matrices.
 *
 * @tparam Scalar The scalar type (e.g., float, double).
 * @tparam Rows Number of rows (Dynamic or fixed).
 * @tparam Cols Number of columns (Dynamic or fixed).
 * @param distance Input dense distance matrix.
 * @param sigma The decay parameter (sigma).
 * @param thresh Threshold to zero-out small weights.
 * @return Eigen::Matrix<Scalar, Rows, Cols> The resulting dense weight matrix.
 */
    template <typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
    Eigen::Matrix<Scalar, Rows, Cols> exponentialKernel(
            const Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>& distance,
            double sigma, double thresh = 1e-6) {

        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        // Compute Exponential kernel: exp(-d / sigma)
        // Using Eigen's expression templates for optimal performance.
        Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> weights =
                (-distance.array() / sigma).exp();

        // Apply thresholding
        if (thresh > 0.0) {
            weights = (weights.array() >= thresh).select(weights, 0.0);
        }

        return weights;
    }

/**
 * @brief Computes the Exponential kernel for Sparse matrices without full dense conversion.
 *
 * This function is memory efficient and avoids converting the entire matrix to dense.
 * It uses sparse-sparse operations and direct value iteration for the exponential.
 *
 * @tparam Scalar The scalar type (e.g., float, double).
 * @tparam Options The storage options (e.g., ColMajor, RowMajor).
 * @tparam StorageIndex The index type (e.g., int).
 * @param distance Input sparse distance matrix.
 * @param sigma The decay parameter (sigma).
 * @param thresh Threshold to zero-out small weights.
 * @return Eigen::SparseMatrix<Scalar, Options, StorageIndex> The resulting sparse weight matrix.
 */
    template <typename Scalar, int Options, typename StorageIndex>
    Eigen::SparseMatrix<Scalar, Options, StorageIndex> exponentialKernel(
            const Eigen::SparseMatrix<Scalar, Options, StorageIndex>& distance,
            double sigma,
            double thresh = 1e-6) {

        // Static assertion to ensure the scalar type is floating point
        static_assert(std::is_floating_point<Scalar>::value,
                      "Scalar type must be floating point (float or double).");

        using SparseMatrixType = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

        // 1. For Exponential kernel, we use the distance directly (no squaring needed).
        // We create a copy to store the result.
        SparseMatrixType resultMatrix = distance;

        // 2. Scale by Exponential factor: Scaled = -1 / sigma
        // Iterate over non-zero elements to apply scaling and exponential
        // This is the most memory-efficient way for the exp() operation on sparse data.
        const Scalar scale_factor = Scalar(-1.0) / Scalar(sigma);

        for (int k = 0; k < resultMatrix.outerSize(); ++k) {
            for (typename SparseMatrixType::InnerIterator it(resultMatrix, k); it; ++it) {
                Scalar val = it.value();
                // Apply scaling: val * (-1 / sigma)
                val *= scale_factor;

                // Apply Exponential: exp(val)
                // Note: exp(0) = 1, so zeros remain zeros (sparsity preserved for 0-distances)
                // However, if distance was 0, exp(0)=1, which creates a new non-zero.
                // This is unavoidable for the diagonal of the graph.
                it.valueRef() = std::exp(val);
            }
        }

        // 3. Apply Thresholding
        if (thresh > 0.0) {
            // prune() removes elements smaller than the reference (thresh)
            // It also removes elements that are exactly zero.
            resultMatrix.prune(thresh);
        }

        return resultMatrix;
    }

//    eigen exponentialKernel(eigen distance, double sigma, double thresh);
//    eigen cauchyKernel(eigen distance, double sigma, double thresh);
//    eigen inverseMultiquadricKernel(eigen distance, double sigma, double thresh);

}

#endif //LIBGSP_KERNELS_H
