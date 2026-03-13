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
        GraphLearningLogDegrees<MatrixType>& setAlpha(Scalar a);

        GraphLearningLogDegrees<MatrixType>& setBeta(Scalar b);

        GraphLearningLogDegrees<MatrixType>& setMaxIterations(int max_it);

        GraphLearningLogDegrees<MatrixType>& setTolerance(Scalar tol);

        GraphLearningLogDegrees<MatrixType>& setStepSize(Scalar step);

        GraphLearningLogDegrees<MatrixType>& setVerbosity(int verb);

        GraphLearningLogDegrees<MatrixType>& setMaxWeight(Scalar max_w);

        // Set Initial W (Must be same type as Z)
        GraphLearningLogDegrees<MatrixType>& setInitialW(const MatrixType &W_init);

        // Set Prior (W0 and c)
        GraphLearningLogDegrees<MatrixType>& setPrior(const MatrixType &W0, Scalar c);

        // Main computation function
        // Input type and Output type are strictly the same (MatrixType)
        MatrixType compute(const MatrixType &Z);
        MatrixType autoCompute(const MatrixType &Z_input, int k);

            // Get statistics
        Stats getStats() const;

    public:
        // Proximal operator for the log-sum barrier (Element-wise on Matrix)
        static MatrixType prox_sum_log(const MatrixType &X, Scalar gamma);
        double calc_theta(const MatrixType& Z_input, int k, bool geom_mean = false, bool is_sorted = false);

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
    };













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
            // We create a copy, zero the diagonal, and return it.
            Derived res = M.derived();
            res.diagonal().setZero();
            return res;
        }

        template<typename Scalar, int Options, typename StorageIndex, typename Derived>
        Derived applyMaskFromZ(
                const Eigen::SparseMatrix<Scalar, Options, StorageIndex> &Z,
                const Eigen::MatrixBase<Derived> &M)
        {
            using SparseMat = Eigen::SparseMatrix<Scalar, Options, StorageIndex>;

            Derived result(Z.rows(), Z.cols());
            result.setZero();

            // Iterate over Z to find the pattern
            for (int k = 0; k < Z.outerSize(); ++k) {
                for (typename SparseMat::InnerIterator it(Z, k); it; ++it) {
                    if (it.row() == it.col()) continue;
                    // Get the value from M at the same position
                    Scalar val = M(it.row(), it.col());
                    // set into result
                    result(it.row(), it.col()) = val;
                }
            }

            return result;
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
                    if (it.row() == it.col()) continue;
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
    } // namespace detail





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
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setAlpha(Scalar a) {
        assert(a > 0 && "Alpha must be positive");
        alpha_ = a;
        return *this;
    }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setBeta(Scalar b) {
        assert(b > 0 && "Beta must be positive");
        beta_ = b;
        return *this;
    }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setMaxIterations(int max_it) { maxit_ = max_it; return *this; }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setTolerance(Scalar tol) { tol_ = tol; return *this; }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setStepSize(Scalar step) { step_size_ = step; return *this; }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setVerbosity(int verb) { verbosity_ = verb; return *this; }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setMaxWeight(Scalar max_w) { max_w_ = max_w; return *this; }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setInitialW(const MatrixType &W_init) {
        W_init_ = W_init;
        has_init_ = true;
        return *this;
    }

    template<typename MatrixType>
    GraphLearningLogDegrees<MatrixType>& GraphLearningLogDegrees<MatrixType>::setPrior(const MatrixType &W0, Scalar c) {
        W0_ = W0;
        prior_c_ = c;
        has_prior_ = true;
        return *this;
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


    template <typename MatrixType>
    double GraphLearningLogDegrees<MatrixType>::calc_theta(const MatrixType& Z_input, int k, bool geom_mean, bool is_sorted) {
        using Scalar = typename MatrixType::Scalar;
        using Index = typename MatrixType::Index;

        Index n = Z_input.rows();

        // Accumulator for the mean calculation
        double sum_theta_inv = 0.0;
        int valid_rows = 0;

        for (Index i = 0; i < n; ++i) {
            std::vector<Scalar> neighbors;

            // --- Extract neighbors (excluding diagonal) ---
            if constexpr (gsp::types::is_eigen_sparse<MatrixType>::value) {
                // Sparse case
                neighbors.reserve(Z_input.nonZeros());
                for (typename MatrixType::InnerIterator it(Z_input, i); it; ++it) {
                    if (it.col() != i) {
                        neighbors.push_back(it.value());
                    }
                }
            } else {
                // Dense case
                neighbors.reserve(n - 1);
                for (Index j = 0; j < n; ++j) {
                    if (i != j) {
                        neighbors.push_back(Z_input(i, j));
                    }
                }
            }

            // --- Find k smallest neighbors ---
            // We need exactly k elements for the calculation (or fewer if not enough neighbors)
            size_t count = std::min(neighbors.size(), static_cast<size_t>(k));

            if (count > 0) {
                // Partial sort to bring the k smallest elements to the front
                std::nth_element(neighbors.begin(), neighbors.begin() + count, neighbors.end());
                // Sort the first 'count' elements to ensure correct order for cumulative sum
                std::sort(neighbors.begin(), neighbors.begin() + count);

                // --- Calculate Theta contribution for this row ---
                double B_k = 0.0; // Cumulative sum of distances
                for (size_t idx = 0; idx < count; ++idx) {
                    Scalar z = neighbors[idx];
                    int K_val = static_cast<int>(idx + 1); // 1-based index for the formula

                    // Formula term: 1 / sqrt(K * z^2 - B * z)
                    double term = K_val * z * z - B_k * z;

                    if (term > 1e-12) { // Numerical stability
                        double val = 1.0 / std::sqrt(term);
                        if (geom_mean) {
                            sum_theta_inv += std::log(val);
                        } else {
                            sum_theta_inv += val;
                        }
                    }
                    B_k += z;
                }
                valid_rows++;
            }
        }

        if (valid_rows == 0) return 0.0;

        // Calculate theta_u (upper bound for the requested k)
        double theta_u;
        if (geom_mean) {
            theta_u = std::exp(sum_theta_inv / valid_rows);
        } else {
            theta_u = sum_theta_inv / valid_rows;
        }

        // Calculate theta_min and theta_max based on the logic in gsp_compute_graph_learning_theta
        // theta_min = theta_u(k)
        // theta_max = theta_u(k) (Note: In the simplified logic provided earlier, bounds were tight)
        // However, strictly following the MATLAB code structure:
        // theta_min = theta_u(k)
        // theta_max = theta_u(k) (since we computed specifically for k)

        double theta_min = theta_u;
        double theta_max = theta_u;

        double theta;
        if (k > 1) {
            theta = std::sqrt(theta_min * theta_max);
        } else {
            theta = theta_min * 1.1;
        }

        return theta;
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
                        if (it.row() == it.col()) continue;
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
                        if (it.row() == it.col()) continue;
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
                        if (it.row() == it.col()) continue;
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

                Y = W - gamma * (grad_h + St_v);
                Y.diagonal().setZero(); // Zero diagonal for Y
                y = v + gamma * (W * ones_vec);

                // Proximal operator f (Positivity + Data term)
                P = Y - 2.0 * gamma * Z_input;
                P = (P.array() < 0).select(0, P); // max(0, P)
                if (max_w_ < std::numeric_limits<Scalar>::infinity()) {
                    P = (P.array() > max_w_).select(max_w_, P);
                }
                P.diagonal().setZero(); // Zero diagonal for P

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



                MatrixType St_p = ones_vec * p.transpose() + p * ones_vec.transpose();
                Q = P - gamma * (grad_h_P + St_p);
                Q.diagonal().setZero(); // Zero diagonal for Q
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


    template<typename MatrixType>
    MatrixType GraphLearningLogDegrees<MatrixType>::autoCompute(const MatrixType &Z_input, int k) {
        setAlpha(1);
        setBeta(1);
        double theta = calc_theta(Z_input, k);
        return compute(Z_input * theta);
    }

} // namespace gsp

#endif //LIBGSP_GRAPHLEARNINGLOGDEGREES_H