// normalized_laplacian_eigen.cpp

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <iostream>
#include <vector>
#include <cmath>

#include "common.h"

// ---- Dense: L_norm = D^{-1/2} (D - W) D^{-1/2} ----
Eigen::MatrixXd computeNormalizedLaplacian(const Eigen::MatrixXd& W) {
    const auto n = W.rows();
    assert(W.cols() == n && "W must be square");

    // degree[i] = sum_j W(i,j)
    Eigen::VectorXd degree = W.rowwise().sum();

    // d_inv_sqrt[i] = 1/sqrt(degree[i]) or 0 if degree[i]==0
    Eigen::VectorXd d_inv_sqrt = degree.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    // L = D - W (dense, no extra temporaries)
    Eigen::MatrixXd L = -W;
    L.diagonal().array() += degree.array();

    // L_norm = D^{-1/2} * L * D^{-1/2} (row/column scaling without building diagonals)
    L = L.array().rowwise() * d_inv_sqrt.transpose().array(); // right scaling (columns)
    L = L.array().colwise() * d_inv_sqrt.array();             // left  scaling (rows)

    return L;
}

// ---- Sparse: L_norm = D^{-1/2} (D - W) D^{-1/2} ----
Eigen::SparseMatrix<double>
computeNormalizedLaplacianSparse(const Eigen::SparseMatrix<double>& W) {
    const int n = static_cast<int>(W.rows());
    assert(W.cols() == n && "W must be square");

    // degree = W * 1
    Eigen::VectorXd one = Eigen::VectorXd::Ones(n);
    Eigen::VectorXd degree = W * one;

    Eigen::VectorXd d_inv_sqrt = degree.unaryExpr(
        [](double x){ return (x > 0.0) ? 1.0/std::sqrt(x) : 0.0; });

    // Build L = D - W via triplets (efficient structural build)
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(static_cast<size_t>(W.nonZeros()) + static_cast<size_t>(n));

    for (int k = 0; k < W.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(W, k); it; ++it) {
            triplets.emplace_back(it.row(), it.col(), -it.value()); // -W
        }
    }
    for (int i = 0; i < n; ++i) {
        triplets.emplace_back(i, i, degree[i]);                     // +D on diag
    }

    Eigen::SparseMatrix<double> L(n, n);
    L.setFromTriplets(triplets.begin(), triplets.end());
    L.makeCompressed();

    // Scale in-place: L_ij *= d_inv_sqrt[i] * d_inv_sqrt[j]
    for (int k = 0; k < L.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(L, k); it; ++it) {
            const int i = it.row();
            const int j = it.col();
            it.valueRef() *= d_inv_sqrt[i] * d_inv_sqrt[j];
        }
    }

    return L;
}

// ---- Example ----
int main() {
    Eigen::MatrixXd W(4,4);
    W << 0,1,1,0,
         1,0,1,0,
         1,1,0,1,
         0,0,1,0;

    tic;
    auto Ldense = computeNormalizedLaplacian(W);
    toc;
    std::cout << "Normalized Laplacian (dense):\n" << Ldense << "\n";

    Eigen::SparseMatrix<double> Ws = W.sparseView();
    tic;
    auto Lsparse = computeNormalizedLaplacianSparse(Ws);
    toc;
    std::cout << "\nNormalized Laplacian (sparse):\n";
    std::cout << Eigen::MatrixXd(Lsparse) << "\n"; // for display

    return 0;
}
