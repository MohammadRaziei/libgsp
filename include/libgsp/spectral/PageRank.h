//
// Created by mohammad on 1/13/26.
//

#ifndef LIBGSP_PAGERANK_H
#define LIBGSP_PAGERANK_H

#include <complex>
#include <Spectra/SymEigsSolver.h>
#include <Spectra/GenEigsSolver.h>

#include "libgsp/Graph.h"
#include "libgsp/iterators/StateMatrixGraph.h"
#include "libgsp/utils/Matrix.h"

#include "libgsp/utils/Logging.h"

namespace gsp {

template <class Matrix>
class PageRankBase {
public:
    explicit PageRankBase(Graph<Matrix>& graph, double p) : PageRankBase(graph, p, "PageRankBase") {}

    virtual gsp::types::densevector_m<Matrix> run() {
        gsp::types::densevector_m<Matrix> eigs;
        int8_t sign = 0;
        auto todouble = [&sign](const std::complex<gsp::types::elem_t<Matrix>> &x) {
            const gsp::types::elem_t<Matrix> y = std::real(x);
            if (sign == 0) { sign = y >= 0 ? 1 : -1; }
            return sign > 0 ? y : -y;
        };
        if constexpr (gsp::types::is_eigen_dense<Matrix>::value) {
            Eigen::EigenSolver<Matrix> solver(matrix_);
            if (solver.info() == Eigen::Success)
                eigs = solver.eigenvectors().col(0).unaryExpr(todouble);
        } else {
            using MatrixD = Eigen::MatrixX<gsp::types::elem_t<Matrix>>;
            MatrixD m = matrix_;
            Eigen::EigenSolver<MatrixD> solver(m);
            if (solver.info() == Eigen::Success)
                eigs = solver.eigenvectors().col(0).unaryExpr(todouble);
        }
        return eigs;
    }

protected:
    explicit PageRankBase(Graph<Matrix>& graph, double p, std::string method) :
            matrix_ (std::move(graph.asymmetricNormalizedWeight())), is_directed_(graph.isDirected()), num_nodes_(graph.numNodes()),
            p_(p), method_(method), logger_(gsp::logging::getLogger(method)) {
        reformMatrix();
    }

    void reformMatrix() {
        gsp::matrix::transposeInplace(matrix_);
        if (p_ == 1 ) return;
        gsp::StateMatrixGraph gen(&matrix_, true, -INFINITY);
        double q = (1 - p_) / matrix_.rows();
        while (auto it = gen.next()) {
            it->setWeight(q + p_ * it->weight());
        }
    }


    Matrix matrix_;
    uint32_t num_nodes_;
    bool is_directed_;
    std::string method_;
    gsp::logging::Logger logger_;
private:
    double p_ = 0.85;
};





template <typename S, int R, int C, int O, int MR, int MC>
class PageRankSpectra : public PageRankBase<Eigen::Matrix<S,R,C,O,MR,MC>> {
public:

    explicit PageRankSpectra(Graph<Eigen::Matrix<S,R,C,O,MR,MC>>& graph, double p, int ncov) :
            PageRankBase<Eigen::Matrix<S,R,C,O,MR,MC>>(graph, p, "PageRankSpectra"),
            op_(this->matrix_), solver_(op_, 1, ncov) {
        solver_.init();
    }

    explicit PageRankSpectra(Graph<Eigen::Matrix<S,R,C,O,MR,MC>>& graph, double p=0.85) : PageRankSpectra(graph, p, 3) {
///             Parameter that controls the convergence speed of the algorithm.
///             Typically a larger `ncv` means faster convergence, but it may
///             also result in greater memory use and more matrix operations
///             in each iteration. This parameter must satisfy \f$nev+2 \le ncv \le n\f$,
///             and is advised to take \f$ncv \ge 2\cdot nev + 1\f$.
    }

    virtual gsp::densevector<S> run() override {
        gsp::densevector<S> eigs;
        int8_t sign = 0;
        auto todouble = [&sign](const std::complex<S> &x) -> S {
            const S y = std::real(x);
            if (sign == 0) { sign = y >= 0 ? 1 : -1; }
            return sign > 0 ? y : -y;
        };

        solver_.compute(Spectra::SortRule::LargestReal);

        if(solver_.info() == Spectra::CompInfo::Successful) {
            eigs = solver_.eigenvectors().unaryExpr(todouble);
        }
        return eigs;
    }

protected:
    Spectra::DenseGenMatProd<S> op_;
    Spectra::GenEigsSolver<Spectra::DenseGenMatProd<S,O>> solver_;
    int ncov_;

};



template <class Matrix>
class PageRankPowerMethod : public PageRankBase<Matrix> {
public:
    explicit PageRankPowerMethod(Graph<Matrix>& graph, double p=0.85) : PageRankBase<Matrix>(graph, p, "PageRankPowerMethod") {

    }
    virtual typename gsp::Graph<Matrix>::densevector run() override {
        return {};

//        x = rand
//                e = inf
//                while e > eps {
//                    y = matrix * x
//                    y = y / norm(y)
//                    e = err(x, y)
//                    x=y
//                }
    }
};

}

#endif //LIBGSP_PAGERANK_H
