// normalized_laplacian_eigen.cpp

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Spectra/SymEigsSolver.h>
#include <unsupported/Eigen/KroneckerProduct>


#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

#include "common.h"
#include "libgsp/iterators/StateMatrixGraph.h"

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


    for (int i = 0; i < n; ++i) {
        double diag_val = degree[i] - W.coeff(i, i); // subtract self-loop
        triplets.emplace_back(i, i, diag_val);
    }
    for (int k = 0; k < W.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(W, k); it; ++it) {
            if (it.row() != it.col()) {
                triplets.emplace_back(it.row(), it.col(), -it.value());
            }
        }
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

double mse(const Eigen::MatrixXd& A, const Eigen::MatrixXd& B) {
    assert(A.rows() == B.rows() && A.cols() == B.cols());
    return (A - B).array().square().mean(); // elementwise squared error mean
}

// ---- Example ----
int main() {
    Eigen::MatrixXd W(4,4);
    W << 0,1,1,0,
         1,0,1,0,
         1,1,0,1,
         0,0,1,0;

    W.transposeInPlace();
    gsp::StateMatrixGraph gen(&W, true, -INFINITY);
    while (auto it = gen.next()) {
        *it += 1;
    }
    std::cout << "W:\n" << W << std::endl;

    gen.reset();
    while (auto it = gen.next()) {
        *it -= 1;
    }

    std::cout << "W:\n" << W << std::endl;


    Eigen::MatrixXd WW = Eigen::KroneckerProduct(W,W);
    std::cout << "WW:\n" << WW << std::endl;


    tic;
    auto Ldense = computeNormalizedLaplacian(W);
    toc;
    std::cout << "Normalized Laplacian (dense):\n" << Ldense << "\n";

    {
        Spectra::DenseSymMatProd<double> op(Ldense);

        // Construct eigen solver object, requesting the largest three eigenvalues
        Spectra::SymEigsSolver<Spectra::DenseSymMatProd<double>> eigs(op, 3, 4);

        // Initialize and compute
        eigs.init();
        int nconv = eigs.compute(Spectra::SortRule::LargestAlge);


        // Retrieve results
        Eigen::VectorXd evalues;
        Eigen::MatrixXd evectors;
        if(eigs.info() == Spectra::CompInfo::Successful) {
            std::cout << "Successfully computed eigenvalues\n";
            evalues = eigs.eigenvalues();
            evectors = eigs.eigenvectors();
        }

        std::cout << "Eigenvalues found:\n" << evalues << std::endl;
        std::cout << "Eigenvectors found:\n" << evectors << std::endl;


    }




    gsp::sparsematrix Ws = W.sparseView();


    Ws = Ws.transpose();
    gsp::StateMatrixGraph gen2(&Ws, true, -INFINITY);
    while (auto it = gen2.next()) {
        *it += 1;
    }
    std::cout << "Ws:\n" << Ws << std::endl;

    gen2.reset();
    while (auto it = gen2.next()) {
        *it -= 1;
    }

    std::cout << "Ws:\n" << Ws << std::endl;



    Eigen::SparseMatrix<double> WWs = Eigen::KroneckerProductSparse(Ws,Ws);
    std::cout << "WWs:\n" << WWs << std::endl;



    tic;
    auto Lsparse = computeNormalizedLaplacianSparse(Ws);
    toc;
    std::cout << "\nNormalized Laplacian (sparse):\n";
    std::cout << Eigen::MatrixXd(Lsparse) << "\n"; // for display


    double error = mse(Ldense, Eigen::MatrixXd(Lsparse));
    std::cout << "\nMSE between dense and sparse L_N: " << error << "\n";


//    auto mat = Ws;
    
//    Eigen::EigenSolver<Eigen::MatrixXd> solver(mat);
//
//    std::cout << "Eigenvalues: \n" << solver.eigenvalues().real() << std::endl;
//
//    std::cout << "Eigenvectors: \n" << solver.eigenvectors().real() << std::endl;


    return 0;
}


