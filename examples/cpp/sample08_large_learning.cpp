//
// Created by mohammad on 3/13/26.
//
#include <iostream>
#include "common.h"
#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/learning/LearningCore.h"
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include <random>

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <Eigen/Dense>
#include <random>

/**
 * Template function to generate synthetic data on an n_features dimensional grid.
 *
 * @tparam Scalar: The data type (e.g., float, double).
 * @param n_sample: Total number of samples (data points).
 * @param n_features: Dimensionality of the data (number of features).
 * @param snr_db: Signal-to-Noise Ratio in dB.
 * @return: An Eigen Matrix with size (n_sample, n_features) of type Scalar.
 */
template <typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> generate_grid_data(int n_sample, int n_features, double snr_db) {
    // Define type aliases for cleaner code
    using MatrixType = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    using VectorType = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;

    // 1. Calculate the number of points per dimension to create a regular grid.
    int points_per_dim = static_cast<int>(std::round(std::pow(n_sample, 1.0 / n_features)));

    if (points_per_dim < 1) points_per_dim = 1;

    // 2. Define Grid Parameters
    // User specified: distance between adjacent points (d) = 1
    Scalar d = static_cast<Scalar>(1.0);

    // Define grid range.
    // Since d=1, if we have 'points_per_dim' points, the total length is (points_per_dim - 1) * d.
    Scalar grid_min = static_cast<Scalar>(0.0);
    Scalar grid_max = grid_min + static_cast<Scalar>(points_per_dim - 1) * d;

    // 3. SNR Conversion (dB to Linear)
    // Formula: SNR_linear = 10^(SNR_db / 10)
    const double snr_linear = std::pow(10.0, snr_db / 10.0);


    // 4. Calculate Noise Variance
    // Formula: noise_var = d^2 / snr_linear
    const Scalar noise_variance = (d * d) / static_cast<Scalar>(snr_linear);
    const Scalar noise_std_dev = std::sqrt(noise_variance);


//    std::cout << "Grid Spacing (d): " << d << std::endl;
//    std::cout << "SNR (dB): " << snr_db << " | SNR (Linear): " << snr_linear << std::endl;
//    std::cout << "Noise Variance: " << noise_variance << std::endl;

    // 5. Generate the Grid Points
    // Adjust n_sample to fit the perfect hypercube grid
    int total_points = static_cast<int>(std::pow(points_per_dim, n_features));
    if (total_points != n_sample) {
        std::cout << "Info: Adjusted n_sample from " << n_sample << " to " << total_points
                  << " to fit a perfect grid." << std::endl;
        n_sample = total_points;
    }

    // Generate coordinates for each dimension using LinSpaced
    // This ensures exact spacing of 'd' between points.
    std::vector<VectorType> coords(n_features);
    for (int i = 0; i < n_features; ++i) {
        coords[i] = VectorType::LinSpaced(points_per_dim, grid_min, grid_max);
    }

    // Create the data matrix
    MatrixType data(n_sample, n_features);

    // Fill the matrix using nested loops logic (Base-N counting)
    Eigen::VectorXi counter(n_features);
    counter.setZero();

    for (int i = 0; i < n_sample; ++i) {
        for (int j = 0; j < n_features; ++j) {
            data(i, j) = coords[j](counter(j));
        }

        // Increment counter
        for (int j = 0; j < n_features; ++j) {
            counter(j)++;
            if (counter(j) < points_per_dim) {
                break;
            } else {
                counter(j) = 0;
            }
        }
    }

    // 6. Add Gaussian Noise (if SNR is not infinite)
    std::mt19937 gen(123); // Initialize generator with fixed seed 123
    // Use standard normal distribution and scale by std_dev
    std::normal_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < n_sample; ++i) {
        for (int j = 0; j < n_features; ++j) {
            // Generate sample and cast to Scalar, then scale
            data(i, j) += static_cast<Scalar>(dist(gen)) * noise_std_dev;
        }
    }


    return data;
}


int main(int argc, char** argv) {
    // Configure logging
    gsp::logging::basicConfig(argc, argv, gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 08 - Large Scale Graph Learning");

    // Define dataset size
    int n_samples = 25;
    int n_features = 2;
    double snr = 20;     // User defined SNR
    int k = n_features * 2;
    int r = 5;
    using Scalar = float;

    // Generate synthetic data
    logger->info("Generating synthetic data: {} samples, {} features", n_samples, n_features);
    tic;
//    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> X(n_samples, n_features); X.setRandom();
    auto X = generate_grid_data<Scalar>(n_samples, n_features, snr);
    toc;
    logger->info("Generating synthetic data: \n{}", fmt::streamed(X));


    // Compute pairwise distances using ANN
    logger->info("Computing sparse distance matrix using ANN (k=10)...");
    tic;
    gsp::NanoflannAnnDistance<Scalar> ann;

    auto sparse_distance = ann.setMetric(gsp::DistanceMetric::L2Distance)
            .setKFixed(k * r)
            .setExcludeSelf(false)
            .setTriangularOnly(false)
            .compute(X);
    toc;

    // Prepare input for learning (Squared distances)
    logger->info("Preparing squared distance matrix Z...");
    tic;
    gsp::sparsematrix_t<Scalar> Z = sparse_distance.cwisePow(2);
    toc;

    // Initialize learner
    gsp::GraphLearningLogDegrees<decltype(Z)> learner;

    // Run optimization
    auto max_iter = 5000;
    logger->info("Starting graph learning optimization (Max Iter: {})...", max_iter);
    tic;
    auto W = learner.setMaxIterations(max_iter)
            .setVerbosity(2)
            .autoCompute(Z, k);
    toc;
//    tic;
//    W.prune([](const Scalar& value, const int& /*row*/, const int& /*col*/) {
//        return std::abs(value) >= 1e-3;
//    });
//    W.makeCompressed();
//    toc;

    // Final log
    logger->info("Optimization finished. Result W shape: {}x{}", W.rows(), W.cols());







// --- Save MATLAB Script ---
    std::ofstream matlabFile("plot_graph.m");
    if (matlabFile.is_open()) {
        matlabFile << "coords = [";
        for (int i = 0; i < X.rows(); ++i) {
            for (int j = 0; j < X.cols(); ++j) {
                matlabFile << X(i, j);
                if (j < X.cols() - 1) matlabFile << " ";
            }
            if (i < X.rows() - 1) matlabFile << ";";
        }
        matlabFile << "];\n\n";

        // Write W as sparse matrix using vectors i, j, s
        // Format: W = sparse(i, j, s, m, n)
        matlabFile << "i = [";
        for (int k = 0; k < W.outerSize(); ++k) {
            for (decltype(W)::InnerIterator it(W, k); it; ++it) {
                matlabFile << it.row() + 1 << ";";
            }
        }
        matlabFile << "];\n";

        matlabFile << "j = [";
        for (int k = 0; k < W.outerSize(); ++k) {
            for (decltype(W)::InnerIterator it(W, k); it; ++it) {
                matlabFile << it.col() + 1 << ";";
            }
        }
        matlabFile << "];\n";

        matlabFile << "s = [";
        for (int k = 0; k < W.outerSize(); ++k) {
            for (decltype(W)::InnerIterator it(W, k); it; ++it) {
                matlabFile << it.value() << ";";
            }
        }
        matlabFile << "];\n\n\n";

        matlabFile << "W = sparse(i, j, s, " << W.rows() << ", " << W.cols() << ");\nW(abs(W) < 1e-3) = 0;\n\n";

        matlabFile << "G = gsp_graph(W, coords);\n";
        matlabFile << "gsp_plot_graph(G);\n";
        matlabFile.close();
        logger->info("MATLAB script saved to plot_graph.m");
    } else {
        logger->error("Failed to open file for writing MATLAB script.");
    }
    return 0;
}