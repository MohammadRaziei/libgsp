// // Created by mohammad on 3/12/26.
//

#ifndef LIBGSP_GRAPHLEARNINGLOGDEGREES_H
#define LIBGSP_GRAPHLEARNINGLOGDEGREES_H

#pragma once

#include "libgsp/utils/Matrix.h"
#include "libgsp/utils/Logging.h"

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <type_traits>


namespace gsp {
// -----------------------------------------------------------------------------
// Namespace for Generic Helper Functions (Masking & Symmetry)
// -----------------------------------------------------------------------------
    namespace detail {


        // -------------------------------------------------------------------------
        // Helper: applyMaskFromZ
        // Applies the structure of Z to matrix M (both must be same type).
        // Dense: Returns M directly.
        // Sparse: Returns M masked by the non-zero pattern of Z.
        // -------------------------------------------------------------------------

        // 1. Dense Version
        template<typename Derived>
        Derived applyMaskFromZ(const Eigen::MatrixBase<Derived> &, const Eigen::MatrixBase<Derived> &M) {
            // For dense matrices, the structure is full.
            // We simply return M (assuming dimensions match).
            return M.derived();
        }

        // 2. Sparse Version
        template<typename Scalar, int Options, typename StorageIndex>
        Eigen::SparseMatrix<Scalar, Options, StorageIndex>
        applyMaskFromZ(const Eigen::SparseMatrix<Scalar, Options, StorageIndex> &Z,
                       const Eigen::SparseMatrix<Scalar, Options, StorageIndex> &M) {

            using SparseMat = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

            SparseMat result(Z.rows(), Z.cols());

            // Reserve memory assuming similar sparsity to Z
            result.reserve(Z.nonZeros());

            // Iterate over Z to find the pattern
            for (int k = 0; k < Z.outerSize(); ++k) {
                for (typename SparseMat::InnerIterator it(Z, k); it; ++it) {
                    // Get the value from M at the same position
                    // coeffRef is efficient here if we are sure the index exists in M,
                    // but to be safe and generic (in case M is slightly different),
                    // we use coeff. However, since M is same type, let's assume structure matches or use coeff.
                    Scalar val = M.coeff(it.row(), it.col());

                    // Insert into result
                    result.insert(it.row(), it.col()) = val;
                }
            }

            return result;
        }

        // -------------------------------------------------------------------------
        // 1. Extract Mask
        //    Creates a binary mask (0 or 1) based on non-zeros of the input matrix.
        //    Handles both Dense and Sparse matrices generically.
        // -------------------------------------------------------------------------

        // Dense Version
        template<typename Derived>
        auto extractMask(const Eigen::MatrixBase<Derived> &mat, double threshold = 0.0) {
            using Scalar = typename Derived::Scalar;
            // Returns a dense matrix of 0s and 1s
            return (mat.array().abs() > threshold).template cast<Scalar>();
        }

        // Sparse Version
        template<typename Scalar, int Options, typename StorageIndex>
        auto extractMask(const Eigen::SparseMatrix<Scalar, Options, StorageIndex> &mat, double threshold = 0.0) {
            using SparseMat = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;
            SparseMat mask(mat.rows(), mat.cols());
            mask.reserve(mat.nonZeros());

            for (int k = 0; k < mat.outerSize(); ++k) {
                for (typename SparseMat::InnerIterator it(mat, k); it; ++it) {
                    if (std::abs(it.value()) > threshold) {
                        mask.insert(it.row(), it.col()) = static_cast<Scalar>(1);
                    }
                }
            }
            mask.makeCompressed();
            return mask;
        }

        // -------------------------------------------------------------------------
        // 2. Apply Mask
        //    Applies the mask to the data matrix (Element-wise multiplication).
        // -------------------------------------------------------------------------

        // Dense Version
        template<typename DerivedA, typename DerivedB>
        void applyMask(const Eigen::MatrixBase<DerivedA> &mask, Eigen::MatrixBase<DerivedB> &data) {
            // data = data .* mask
            data.const_cast_derived() = data.cwiseProduct(mask);
        }

        // Sparse Version
        template<typename Scalar, int Options, typename StorageIndex>
        void applyMask(const Eigen::SparseMatrix<Scalar, Options, StorageIndex> &mask,
                       Eigen::SparseMatrix<Scalar, Options, StorageIndex> &data) {
            // Efficiently zero out elements in 'data' where 'mask' is zero.
            // Since we assume mask structure matches data structure (or is a subset),
            // we iterate over data and check mask.

            data.makeCompressed();
            for (int k = 0; k < data.outerSize(); ++k) {
                for (typename Eigen::SparseMatrix<Scalar, Options, StorageIndex>::InnerIterator it(data, k); it; ++it) {
                    // If mask is zero at this position, remove edge from W
                    if (mask.coeff(it.row(), it.col()) == 0) {
                        it.valueRef() = 0;
                    }
                }
            }
            data.prune(0, 0); // Remove explicit zeros
        }

        // -------------------------------------------------------------------------
        // 3. Enforce Symmetry
        //    W = (W + W') / 2
        // -------------------------------------------------------------------------

        // Dense Version
        template<typename Derived>
        void enforceSymmetry(Eigen::MatrixBase<Derived> &mat) {
            mat.const_cast_derived() = (mat + mat.transpose()).eval() * 0.5;
        }

        // Sparse Version
        template<typename Scalar, int Options, typename StorageIndex>
        void enforceSymmetry(Eigen::SparseMatrix<Scalar, Options, StorageIndex> &mat) {
            // For sparse, we add the transpose and then prune.
            // Note: This might slightly change the sparsity pattern (fill-in).
            mat = mat + mat.transpose();
            mat *= 0.5;
            mat.prune(0, 0); // Clean up near-zeros created by addition
        }

    } // namespace detail

// -----------------------------------------------------------------------------
// Main Class Definition
// -----------------------------------------------------------------------------

    template<typename MatrixType>
    class GraphLearningLogDegrees {
    public:
        using Scalar = typename std::decay_t<MatrixType>::Scalar;
        using Index = typename std::decay_t<MatrixType>::Index;

        // Structure to hold optimization statistics
        struct Stats {
            std::vector<double> f_eval;
            std::vector<double> g_eval;
            std::vector<double> h_eval;
            std::vector<double> total_eval;
            std::vector<double> pos_violation;
            double time = 0.0;
            int iterations = 0;
        };

        // Constructor
        GraphLearningLogDegrees();

        // Setters for parameters
        void setAlpha(Scalar a);

        void setBeta(Scalar b);

        void setMaxIterations(int max_it);

        void setTolerance(Scalar tol);

        void setStepSize(Scalar step);

        void setVerbosity(int verb);

        void setMaxWeight(Scalar max_w);

        // Set Initial W (Must be same type as Z)
        void setInitialW(const MatrixType &W_init);

        // Set Prior (W0 and c)
        void setPrior(const MatrixType &W0, Scalar c);

        // Main computation function
        // Input type and Output type are strictly the same (MatrixType)
        MatrixType compute(const MatrixType &Z);

        // Get statistics
        Stats getStats() const;

    private:
        gsp::logging::Logger logger_ = gsp::logging::getLogger("GraphLearningLogDegrees");
        // Parameters
        Scalar alpha_;
        Scalar beta_;
        uint32_t maxit_;
        Scalar tol_;
        Scalar step_size_;
        int verbosity_;
        Scalar max_w_;

        // Prior parameters
        bool has_prior_;
        Scalar prior_c_;
        MatrixType W0_;

        // Internal state
        bool has_init_;
        MatrixType W_init_;
        Stats stats_;

        // Helper functions
        Scalar lin_map(Scalar x, Scalar out_min, Scalar out_max, Scalar in_min, Scalar in_max);

        // Proximal operator for the log-sum barrier (Element-wise on Matrix)
        MatrixType prox_sum_log(const MatrixType &X, Scalar gamma);
    };

// -----------------------------------------------------------------------------
// Implementation Section
// -----------------------------------------------------------------------------

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>::GraphLearningLogDegrees()
            : alpha_(1.0), beta_(1.0), maxit_(1000), tol_(1e-5),
              step_size_(0.5), verbosity_(1), max_w_(std::numeric_limits<Scalar>::infinity()),
              has_prior_(false), prior_c_(0.0), has_init_(false) {}

// Setters
    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setAlpha(Scalar a) { alpha_ = a; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setBeta(Scalar b) { beta_ = b; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setMaxIterations(int max_it) { maxit_ = max_it; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setTolerance(Scalar tol) { tol_ = tol; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setStepSize(Scalar step) { step_size_ = step; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setVerbosity(int verb) { verbosity_ = verb; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setMaxWeight(Scalar max_w) { max_w_ = max_w; }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setInitialW(const MatrixType &W_init) {
        W_init_ = W_init;
        has_init_ = true;
    }

    template<typename MatrixType>
    void GraphLearningLogDegrees<MatrixType>::setPrior(const MatrixType &W0, Scalar c) {
        W0_ = W0;
        prior_c_ = c;
        has_prior_ = true;
    }

    template<typename MatrixType>
    typename GraphLearningLogDegrees<MatrixType>::Stats GraphLearningLogDegrees<MatrixType>::getStats() const {
        return stats_;
    }

    template<typename MatrixType>
    typename GraphLearningLogDegrees<MatrixType>::Scalar
    GraphLearningLogDegrees<MatrixType>::lin_map(Scalar x, Scalar out_min, Scalar out_max, Scalar in_min,
                                                 Scalar in_max) {
        return (x - in_min) * ((out_max - out_min) / (in_max - in_min)) + out_min;
    }

// Proximal operator for log-sum barrier applied element-wise to the matrix
    template<typename MatrixType>
    MatrixType GraphLearningLogDegrees<MatrixType>::prox_sum_log(const MatrixType &X, Scalar gamma) {
        // sol = (x + sqrt(x.^2 + 4*gamma)) / 2;
        // We use Eigen array operations for genericity (works for both Dense and Sparse)
        return (X.array().square() + 4.0 * gamma).sqrt().cwiseQuotient(2.0).cwiseQuotient(2.0) + X.array() / 2.0;
        // Note: X/2 + sqrt(X^2 + 4g)/2
    }

    template<typename MatrixType>
    MatrixType GraphLearningLogDegrees<MatrixType>::compute(const MatrixType &Z_input) {
        auto start_time = std::chrono::high_resolution_clock::now();
        Index n = Z_input.rows();

        // Initialize W
        MatrixType W;
        if (has_init_) {
            W = W_init_;
        } else {
            // Initialize with zeros respecting the structure of Z
            W = Z_input * 0.0;
        }

        // Prior handling
        MatrixType W0_mat(n, n);
        if (has_prior_) {
            W0_mat = detail::applyMaskFromZ(Z_input, W0_);
        } else {
            W0_mat.setZero();
        }

        // 2. Optimization Parameters
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ones_vec = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>::Ones(n);

        // Calculate norm_K (upper bound for the operator W -> W*ones)
        Scalar norm_K = std::sqrt(static_cast<Scalar>(n));

        Scalar h_beta = 2.0 * (beta_ + (has_prior_ ? prior_c_ : 0.0));
        Scalar mu = h_beta + norm_K;

        Scalar epsilon = lin_map(0.0, 0.0, 1.0 / (1.0 + mu), 0.0, 1.0);
        Scalar gamma = lin_map(step_size_, epsilon, (1.0 - epsilon) / mu, 0.0, 1.0);

        // Initialize dual variable v (Node degrees)
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> v = W * ones_vec;

        stats_.f_eval.clear();
        stats_.g_eval.clear();
        stats_.h_eval.clear();
        stats_.total_eval.clear();
        stats_.pos_violation.clear();

        if (verbosity_ > 1) {
            logger_->info("Starting optimization... Nodes: {}", n);
        }

        MatrixType grad_h;

        MatrixType Y, P, Q;
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> y, p, q;

        // 3. Main Loop (FBF Algorithm adapted for Matrices)
        for (int iter = 0; iter < maxit_; ++iter) {
            // Gradient of h(W) = beta*||W||^2 + c/2*||W-W0||^2
            // grad_h = 2*(beta+c)*W - c*W0
            if (has_prior_) {
                grad_h = 2.0 * ((beta_ + prior_c_) * W - prior_c_ * W0_mat);
            } else {
                grad_h = 2.0 * (beta_ * W);
            }

            // ---------------------------------------------------------
            // Compute Y, P, Q based on MatrixType (Dense vs Sparse)
            // ---------------------------------------------------------

            if constexpr (gsp::types::is_eigen_sparse<MatrixType>::value) {
                // --- CASE: SPARSE (Memory Efficient) ---

                // 1. Compute Y = W - gamma * (grad_h + St_v)
                Y = W;
                for (int k = 0; k < Y.outerSize(); ++k) {
                    for (typename MatrixType::InnerIterator it(Y, k); it; ++it) {
                        Scalar val = it.value();
                        Scalar grad_val = grad_h.coeff(it.row(), it.col());
                        Scalar st_v_val = v(it.row()) + v(it.col());
                        it.valueRef() = val - gamma * (grad_val + st_v_val);
                    }
                }

                // 2. Compute P = Prox_f(Y)
                P = Y - 2.0 * gamma * Z_input;
                for (int k = 0; k < P.outerSize(); ++k) {
                    for (typename MatrixType::InnerIterator it(P, k); it; ++it) {
                        Scalar val = it.value();
                        if (val < 0) val = 0;
                        else if (val > max_w_) val = max_w_;
                        it.valueRef() = val;
                    }
                }

                // 3. Compute y = v + gamma * (S * W)
                y = v + gamma * (W * ones_vec);

                // 4. Proximal operator g_star (Log barrier on degrees)
                Scalar inner_gamma = 1.0 / (gamma * alpha_);
                Eigen::Matrix<Scalar, Eigen::Dynamic, 1> prox_arg = y / (gamma * alpha_);

                Eigen::Matrix<Scalar, Eigen::Dynamic, 1> g_prox_val =
                        (prox_arg.array().square() + 4.0 * inner_gamma).sqrt() / 2.0 + prox_arg.array() / 2.0;

                p = y - gamma * alpha_ * g_prox_val;

                // 5. Compute Q = P - gamma * (grad_h_P + St_p)
                MatrixType grad_h_P;

                if (has_prior_) {
                    grad_h_P = 2.0 * ((beta_ + prior_c_) * P - prior_c_ * W0_mat);
                } else {
                    grad_h_P = 2.0 * (beta_ * P);
                }


                Q = P;
                for (int k = 0; k < Q.outerSize(); ++k) {
                    for (typename MatrixType::InnerIterator it(Q, k); it; ++it) {
                        Scalar val = it.value();
                        Scalar grad_val = grad_h_P.coeff(it.row(), it.col());
                        Scalar st_p_val = p(it.row()) + p(it.col());
                        it.valueRef() = val - gamma * (grad_val + st_p_val);
                    }
                }

                // 6. Compute q = p + gamma * (S * P)
                q = p + gamma * (P * ones_vec);

            } else {
                // --- CASE: DENSE (Fast & Vectorized) ---

                MatrixType St_v = ones_vec * v.transpose() + v * ones_vec.transpose();
                MatrixType St_p = ones_vec * p.transpose() + p * ones_vec.transpose();

                Y = W - gamma * (grad_h + St_v);
                y = v + gamma * (W * ones_vec);

                // Proximal operator f (Positivity + Data term)
                P = Y - 2.0 * gamma * Z_input;
                P = (P.array() < 0).select(0, P); // max(0, P)
                if (max_w_ < std::numeric_limits<Scalar>::infinity()) {
                    P = (P.array() > max_w_).select(max_w_, P);
                }

                // Proximal operator g_star (Log barrier on degrees)
                Scalar inner_gamma = 1.0 / (gamma * alpha_);
                Eigen::Matrix<Scalar, Eigen::Dynamic, 1> prox_arg = y / (gamma * alpha_);

                Eigen::Matrix<Scalar, Eigen::Dynamic, 1> g_prox_val =
                        (prox_arg.array().square() + 4.0 * inner_gamma).sqrt() / 2.0 + prox_arg.array() / 2.0;

                p = y - gamma * alpha_ * g_prox_val;

                // Second gradient step
                MatrixType grad_h_P;

                if (has_prior_) {
                    grad_h_P = 2.0 * ((beta_ + prior_c_) * P - prior_c_ * W0_mat);
                } else {
                    grad_h_P = 2.0 * (beta_ * P);
                }



                St_p = ones_vec * p.transpose() + p * ones_vec.transpose();
                Q = P - gamma * (grad_h_P + St_p);
                q = p + gamma * (P * ones_vec);
            }

            // Compute relative changes
            Scalar rel_norm_primal = (-Y + Q).norm() / (W.norm() + 1e-10);
            Scalar rel_norm_dual = (-y + q).norm() / (v.norm() + 1e-10);

            // Update variables
            W = W - Y + Q;
            v = v - y + q;

            // Logging
            if (verbosity_ > 1) {
                logger_->info("Iter {}: Primal {:.6e}, Dual {:.6e}", iter, rel_norm_primal, rel_norm_dual);
            }

            // Check convergence
            if (rel_norm_primal < tol_ && rel_norm_dual < tol_) {
                if (verbosity_ > 0) {
                    logger_->info("Converged at iteration {}", iter);
                }
                stats_.iterations = iter + 1;
                break;
            }
            stats_.iterations = iter + 1;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        stats_.time = std::chrono::duration<double>(end_time - start_time).count();

        if (verbosity_ > 0) {
            logger_->info("Finished. Time: {:.3f}s", stats_.time);
        }

        return W;
    }

} // namespace gsp

#endif //LIBGSP_GRAPHLEARNINGLOGDEGREES_H