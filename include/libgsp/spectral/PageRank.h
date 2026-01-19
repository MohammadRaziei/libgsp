//
// Created by mohammad on 1/13/26.
//

#ifndef LIBGSP_PAGERANK_H
#define LIBGSP_PAGERANK_H

#include <complex>
#include <limits>

//#include <Eigen/>
#include <Spectra/GenEigsSolver.h>
#include <Spectra/MatOp/SparseGenMatProd.h>

#include "libgsp/Graph.h"
#include "libgsp/iterators/StateMatrixGraph.h"
#include "libgsp/utils/Matrix.h"

#include "libgsp/utils/Logging.h"

namespace gsp {

template <class Matrix>
class PageRankBase {
public:
    explicit PageRankBase(Graph<Matrix>& graph, double p) : PageRankBase(graph, p, "PageRankBase") {}

    [[nodiscard]] std::string method() const { return method_; }

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

template <class Matrix>
class PageRankPowerMethod : public PageRankBase<Matrix> {
public:
    explicit PageRankPowerMethod(Graph<Matrix>& graph, double p=0.85) : PageRankBase<Matrix>(graph, p, "PageRankPowerMethod") {
    }
    virtual gsp::types::densevector_m<Matrix> run() override {
        gsp::types::densevector_m<Matrix> x { Eigen::MatrixXd::Random(this->num_nodes_, 1) }, y;
        error = INFINITY;
        iter_count = 0;

        while (error > eps && iter_count < max_number_iter) {
            ++iter_count;
            y = this->matrix_ * x;
            y /= y.norm();
            error = (y-x).norm();
            x = y;
        }
        this->logger_->debug("stop after {} iteration with error: {}", iter_count, error);

        return x;
    }
    double eps = 1e-10;
    uint32_t max_number_iter = std::numeric_limits<uint32_t>::max();

    double error = INFINITY;
    uint32_t iter_count = 0;
};



template <typename Matrix>
class PageRankSpectra : public PageRankBase<Matrix> {
public:

    using MatProd = std::conditional_t<types::is_eigen_dense<Matrix>::value,
        Spectra::DenseGenMatProd<typename types::is_eigen_dense<Matrix>::scalar, types::is_eigen_dense<Matrix>::options>,
            std::conditional_t<types::is_eigen_sparse<Matrix>::value,
                Spectra::SparseGenMatProd<typename types::is_eigen_sparse<Matrix>::scalar, types::is_eigen_sparse<Matrix>::options>,
                    void>>;

    explicit PageRankSpectra(Graph<Matrix>& graph, double p, int ncov) :
            PageRankBase<Matrix>(graph, p, "PageRankSpectra"),
            op_(this->matrix_), ncov_(ncov)  {
        solver_ = std::make_shared<Spectra::GenEigsSolver<MatProd>>(op_, 1, ncov_);
        solver_->init();
    }

    explicit PageRankSpectra(Graph<Matrix>& graph, double p=0.85) : PageRankSpectra(graph, p, 3) {
///             Parameter that controls the convergence speed of the algorithm.
///             Typically a larger `ncv` means faster convergence, but it may
///             also result in greater memory use and more matrix operations
///             in each iteration. This parameter must satisfy \f$nev+2 \le ncv \le n\f$,
///             and is advised to take \f$ncv \ge 2\cdot nev + 1\f$.
    }

    virtual gsp::types::densevector_m<Matrix> run() override {
        gsp::types::densevector_m<Matrix> eigs;
        int8_t sign = 0;
        auto todouble = [&sign](const std::complex<gsp::types::elem_t<Matrix>> &x) {
            const gsp::types::elem_t<Matrix> y = std::real(x);
            if (sign == 0) { sign = y >= 0 ? 1 : -1; }
            return sign > 0 ? y : -y;
        };

        solver_->compute(Spectra::SortRule::LargestReal);

        if(solver_->info() == Spectra::CompInfo::Successful) {
            eigs = solver_->eigenvectors().unaryExpr(todouble);
        }
        return eigs;
    }

protected:
    MatProd op_;
    std::shared_ptr<Spectra::GenEigsSolver<MatProd>> solver_;
    int ncov_;
};


enum class PageRankMethod {
    Base=0,
    PowerMethod,
    Spectra,
};
template <class Matrix>
class PageRank {
public:
    explicit PageRank(Graph<Matrix>& graph, double p=0.85, PageRankMethod method = PageRankMethod::PowerMethod) {
        PageRankBase<Matrix>* pr = nullptr;
        switch (method) {
            case PageRankMethod::Base: pr = new PageRankBase<Matrix>(graph, p); break;
            case PageRankMethod::PowerMethod: pr = new PageRankPowerMethod<Matrix>(graph, p); break;
            case PageRankMethod::Spectra: pr = new PageRankSpectra<Matrix>(graph, p); break;
        }
        page_rank_method_ = std::move(std::shared_ptr<PageRankBase<Matrix>>(pr));
    }
    gsp::types::densevector_m<Matrix> run() {
        return page_rank_method_->run();
    }

    [[nodiscard]] std::string method() const { return page_rank_method_->method(); }

private:
    std::shared_ptr<PageRankBase<Matrix>> page_rank_method_;
};
}

#endif //LIBGSP_PAGERANK_H
