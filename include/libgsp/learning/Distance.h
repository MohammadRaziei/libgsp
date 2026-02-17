//
// Created by mohammad on 2/5/26.
//

#ifndef LIBGSP_DISTANCE_H
#define LIBGSP_DISTANCE_H
#pragma once

#include "libgsp/utils/Types.h"

#include <Eigen/Dense>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <nanoflann.hpp>

#include <stdexcept>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <memory>

namespace gsp {

// ------------------------------------------------------------
// pairwiseDistance (row-wise):
// D_ij = sqrt( (x_i - y_j)^T P (x_i - y_j) )
// X: n x d (rows are vectors), Y: m x d, P: d x d
// Returns: n x m (distances, not squared)
// ------------------------------------------------------------
    template <
            class ScalarX, int RX, int CX, int OptX, int MRX, int MCX,
            class ScalarY, int RY, int CY, int OptY, int MRY, int MCY,
            class ScalarP, int RP, int CP, int OptP, int MRP, int MCP
    >
    auto pairwiseQuadraticDistance(
            const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X,
            const Eigen::Matrix<ScalarY, RY, CY, OptY, MRY, MCY>& Y,
            const Eigen::Matrix<ScalarP, RP, CP, OptP, MRP, MCP>& P
    )
    -> Eigen::Matrix<
            gsp::types::float_of<ScalarX>,
            RX,  // n
            RY,  // m
            OptX,
            MRX, // max n
            MRY  // max m
    >
    {
        using OutScalar = gsp::types::float_of<ScalarX>;
        using OutMat    = Eigen::Matrix<OutScalar, RX, RY, OptX, MRX, MRY>;

        // X: nxd, Y: mxd  => same d
        if (X.cols() != Y.cols()) throw std::invalid_argument("pairwiseDistance: X.cols != Y.cols");
        if (P.rows() != P.cols()) throw std::invalid_argument("pairwiseDistance: P not square");
        if (P.rows() != X.cols()) throw std::invalid_argument("pairwiseDistance: P.rows != X.cols");

        const Eigen::Index n = X.rows();
        const Eigen::Index m = Y.rows();

        const auto Xf = X.template cast<OutScalar>();
        const auto Yf = Y.template cast<OutScalar>();
        const auto Pf = P.template cast<OutScalar>();

        // XP = X*P, YP = Y*P
        const Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> XP = Xf * Pf; // n x d

        // a_i = x_i^T P x_i  (n x 1)
        const Eigen::Matrix<OutScalar, Eigen::Dynamic, 1> a =
                (Xf.array() * XP.array()).rowwise().sum().matrix();

        // b_j = y_j^T P y_j  (m x 1)
        const Eigen::Matrix<OutScalar, Eigen::Dynamic, 1> b =
                (Yf.array() * (Yf * Pf).array()).rowwise().sum().matrix();

        // C = X P Y^T  (n x m)
        const Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> C =
                XP * Yf.transpose();

        OutMat S;
        S.resize(n, m);

        // S_ij = a_i + b_j - 2*C_ij  (robust broadcasting)
        S = a.replicate(1, m);
        S.rowwise() += b.transpose();
        S.noalias() -= OutScalar(2) * C;

        S = S.cwiseMax(OutScalar(0));
        return S.cwiseSqrt();
    }

// ------------------------------------------------------------
// Euclidean overloads (P = I), row-wise
// ------------------------------------------------------------
    template <
            class ScalarX, int RX, int CX, int OptX, int MRX, int MCX,
            class ScalarY, int RY, int CY, int OptY, int MRY, int MCY
    >
    auto pairwiseL2Distance(
            const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X,
            const Eigen::Matrix<ScalarY, RY, CY, OptY, MRY, MCY>& Y
    )
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RY, OptX, MRX, MRY>
    {
        using OutScalar = gsp::types::float_of<ScalarX>;
        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> I =
                Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic>::Identity(X.cols(), X.cols());
        return pairwiseQuadraticDistance(X, Y, I);
    }

    template <class ScalarX, int RX, int CX, int OptX, int MRX, int MCX>
    auto pairwiseL2Distance(const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X)
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RX, OptX, MRX, MRX>
    {
        return pairwiseL2Distance(X, X);
    }



    // ------------------------------------------------------------
// pairwiseCosineDistance (row-wise):
// Cosine distance = 1 - cosine_similarity
// cosine_similarity(x,y) = (x·y) / (||x|| ||y||)
//
// X: n x d (rows are vectors), Y: m x d
// Returns: n x m
//
// Notes:
// - If a row has zero norm, we define its cosine similarity with anything as 0,
//   hence cosine distance = 1.
// - Numerically clamps similarity to [-1, 1].
// ------------------------------------------------------------
    template <
            class ScalarX, int RX, int CX, int OptX, int MRX, int MCX,
            class ScalarY, int RY, int CY, int OptY, int MRY, int MCY
    >
    auto pairwiseChordalDistance(
            const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X,
            const Eigen::Matrix<ScalarY, RY, CY, OptY, MRY, MCY>& Y
    )
    -> Eigen::Matrix<
            gsp::types::float_of<ScalarX>,
            RX,
            RY,
            OptX,
            MRX,
            MRY
    >
    {
        using OutScalar = gsp::types::float_of<ScalarX>;
        using OutMat    = Eigen::Matrix<OutScalar, RX, RY, OptX, MRX, MRY>;

        if (X.cols() != Y.cols())
            throw std::invalid_argument("pairwiseCosineDistance: X.cols != Y.cols");

        const Eigen::Index n = X.rows();
        const Eigen::Index m = Y.rows();

        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> Xn =
                X.template cast<OutScalar>();
        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> Yn =
                Y.template cast<OutScalar>();

        // Normalize rows (zero stays zero)
        for (Eigen::Index i = 0; i < n; ++i) {
            const OutScalar nx = Xn.row(i).norm();
            if (nx > OutScalar(0))
                Xn.row(i) /= nx;
        }
        for (Eigen::Index j = 0; j < m; ++j) {
            const OutScalar ny = Yn.row(j).norm();
            if (ny > OutScalar(0))
                Yn.row(j) /= ny;
        }

        OutMat D;
        D.resize(n, m);

        // D_ij = 0.5 * ||x_i - y_j||^2
        for (Eigen::Index i = 0; i < n; ++i) {
            for (Eigen::Index j = 0; j < m; ++j) {
                D(i, j) = OutScalar(0.5) *
                          (Xn.row(i) - Yn.row(j)).squaredNorm();
            }
        }

        return D;
    }

    // Convenience overload: X vs X
    template <class ScalarX, int RX, int CX, int OptX, int MRX, int MCX>
    auto pairwiseChordalDistance(const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X)
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RX, OptX, MRX, MRX>
    {
        return pairwiseChordalDistance(X, X);
    }

    /////////////////////////////

    template <
            class ScalarX, int RX, int CX, int OptX, int MRX, int MCX,
            class ScalarY, int RY, int CY, int OptY, int MRY, int MCY
    >
    auto pairwiseCosineSimilarity(
            const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X,
            const Eigen::Matrix<ScalarY, RY, CY, OptY, MRY, MCY>& Y
    )
    -> Eigen::Matrix<
            gsp::types::float_of<ScalarX>,
            RX,
            RY,
            OptX,
            MRX,
            MRY
    >
    {
        using OutScalar = gsp::types::float_of<ScalarX>;
        using OutMat    = Eigen::Matrix<OutScalar, RX, RY, OptX, MRX, MRY>;

        if (X.cols() != Y.cols())
            throw std::invalid_argument("pairwiseCosineSimilarity: X.cols != Y.cols");

        const Eigen::Index n = X.rows();
        const Eigen::Index m = Y.rows();

        // Cast for stable computation
        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> Xn = X.template cast<OutScalar>();
        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> Yn = Y.template cast<OutScalar>();

        // Normalize rows (zero stays zero)
        for (Eigen::Index i = 0; i < n; ++i) {
            const OutScalar nx = Xn.row(i).norm();
            if (nx > OutScalar(0)) Xn.row(i) /= nx;
        }
        for (Eigen::Index j = 0; j < m; ++j) {
            const OutScalar ny = Yn.row(j).norm();
            if (ny > OutScalar(0)) Yn.row(j) /= ny;
        }

        // Cosine similarity = <x_hat, y_hat>
        OutMat S;
        S.resize(n, m);
        S.noalias() = Xn * Yn.transpose();

        // Optional: clamp for numeric drift
//        S = S.cwiseMin(OutScalar(1)).cwiseMax(OutScalar(-1));

        return S;
    }


    ////////////////////////////////

    // Convenience overload: X vs X
    template <class ScalarX, int RX, int CX, int OptX, int MRX, int MCX>
    auto pairwiseCosineSimilarity(const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X)
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RX, OptX, MRX, MRX>
    {
        return pairwiseCosineSimilarity(X, X);
    }



// Output distance metric
// - L2: Euclidean distance
// - Cosine: cosine distance = 1 - cosine_similarity
    enum class DistanceMetric { L2Distance, ChordalDistance };

// ============================================================
// BaseKnnDistance (abstract contract)
// - Only what we are sure we always need: build + compute (+ basic shape getters).
// ============================================================
    template <class Scalar>
    class BaseKnnDistance {
    public:
        using ScalarF  = gsp::types::float_of<Scalar>;
        using Dense  = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
        using DenseF  = Eigen::Matrix<ScalarF, Eigen::Dynamic, Eigen::Dynamic>;
        using DenseCRef = Eigen::Ref<const Dense>;
        using Sparse = Eigen::SparseMatrix<ScalarF>;

        virtual ~BaseKnnDistance() = default;

        // Build an internal index / store Y (m x d)
        virtual void build(DenseCRef y) = 0;

        // Compute sparse distances between X (n x d) and built Y (m x d)
        // Output values are distances (not weights).
        virtual Sparse compute(DenseCRef x, bool compute_self = false) const = 0;

        virtual Sparse compute() const {
            if (this->y_.size() == 0)
                throw std::runtime_error("KnnDistance::compute: call build(Y) first.");
            return compute(this->y_, true);
        }

        virtual Sparse operator()() const { return compute(); }
        virtual Sparse operator()(DenseCRef x) const { return compute(x); }


        // Implementation of BaseKnnDistance pure virtual functions
        virtual uint32_t dim() const {
            if (y_.size() == 0)
                throw std::runtime_error("KnnDistance::dim: call build(Y) first.");
            return static_cast<uint32_t>(y_.cols());
        }

        virtual uint32_t size() const {
            if (y_.size() == 0)
                throw std::runtime_error("KnnDistance::size: call build(Y) first.");
            return static_cast<uint32_t>(y_.rows());
        }

    protected:
        DenseF y_;
    };

// ============================================================
// Helpers
// ============================================================
    namespace detail {

        template <class Mat>
        inline void normalizeRowsInplace(Mat& a) {
            using Scalar = typename Mat::Scalar;
            for (Eigen::Index i = 0; i < a.rows(); ++i) {
                const Scalar nrm = a.row(i).norm();
                if (nrm > Scalar(0)) {
                    a.row(i) /= nrm;
                } else {
                    a.row(i).setZero();  // explicit zero-vector handling
                }
            }
        }

        inline int effectiveK(int k_fixed, double k_per_dim, int d) {
            const int kk = (k_fixed > 0) ? k_fixed : static_cast<int>(std::ceil(k_per_dim * double(d)));
            return std::max(1, kk);
        }

    } // namespace detail

// ============================================================
// KnnDistance (exact kNN via brute-force)
// - build(): stores Y (and optionally normalized Y for cosine).
// - compute(): for each row of X, scans all Y, selects k smallest distances.
// ============================================================
    template <class Scalar>
    class KnnDistance : public BaseKnnDistance<Scalar> {
    public:
        using ScalarF   = typename BaseKnnDistance<Scalar>::ScalarF;
        using Dense     = typename BaseKnnDistance<Scalar>::Dense;
        using DenseF    = typename BaseKnnDistance<Scalar>::DenseF;
        using DenseCRef = typename BaseKnnDistance<Scalar>::DenseCRef;
        using Sparse    = typename BaseKnnDistance<Scalar>::Sparse;

        using BaseKnnDistance<Scalar>::compute;

        KnnDistance& setMetric(DistanceMetric metric) {
            metric_ = metric;
            return *this;
        }

        KnnDistance& setKFixed(int k_fixed) {
            k_fixed_ = k_fixed;
            return *this;
        }

        KnnDistance& setKPerDim(double k_per_dim) {
            k_per_dim_ = k_per_dim;
            return *this;
        }

        KnnDistance& setTriangularOnly(bool triangular_only) {
            triangular_only_ = triangular_only;
            return *this;
        }

        KnnDistance& setExcludeSelf(bool exclude_self) {
            exclude_self_ = exclude_self;
            return *this;
        }

        void build(DenseCRef y) {
            if (y.rows() <= 0 || y.cols() <= 0)
                throw std::invalid_argument("KnnDistance::build: empty Y");

            this->y_ = y;

            // Normalize only for ChordalDistance (unit-sphere distance)
            if (metric_ == DistanceMetric::ChordalDistance) {
                detail::normalizeRowsInplace(this->y_);
            }
        }

        Sparse compute(DenseCRef x, bool compute_self = false) const override {
            if (this->y_.size() == 0)
                throw std::runtime_error("KnnDistance::compute: call build(Y) first.");

            if (compute_self && x.rows() != this->y_.rows())
                throw std::invalid_argument("compute_self requires X and Y to have same number of rows");

            if (x.cols() != this->y_.cols())
                throw std::invalid_argument("KnnDistance::compute: X.cols != Y.cols");

            const int n = static_cast<int>(x.rows());
            const int m = static_cast<int>(this->y_.rows());
            const int d = static_cast<int>(x.cols());
            const int k = std::min(effectiveK(d), m);

            Dense x_work = x;

            // Normalize only for ChordalDistance to match pairwiseChordalDistance/ANN behavior
            if (metric_ == DistanceMetric::ChordalDistance) {
                detail::normalizeRowsInplace(x_work);
            }

            std::vector<Eigen::Triplet<ScalarF>> triplets;
            triplets.reserve(static_cast<size_t>(n) * static_cast<size_t>(k));

            std::vector<int> indices(m);
            std::iota(indices.begin(), indices.end(), 0);

            for (int i = 0; i < n; ++i) {
                std::vector<ScalarF> dist(m);

                if (metric_ == DistanceMetric::L2Distance) {
                    for (int j = 0; j < m; ++j) {
                        const ScalarF d2 =
                                (x_work.row(i) - this->y_.row(j)).squaredNorm();
                        dist[j] = std::sqrt(std::max<ScalarF>(ScalarF(0), d2));
                    }
                } else if (metric_ == DistanceMetric::ChordalDistance) {
                    // d = 0.5 * ||x_hat - y_hat||^2  (exactly what pairwiseChordalDistance does)
                    for (int j = 0; j < m; ++j) {
                        const ScalarF d2 =
                                (x_work.row(i) - this->y_.row(j)).squaredNorm();
                        dist[j] = ScalarF(0.5) * std::max<ScalarF>(ScalarF(0), d2);
                    }
                } else {
                    throw std::invalid_argument("KnnDistance::compute: unsupported metric");
                }

                std::nth_element(
                        indices.begin(),
                        indices.begin() + k,
                        indices.end(),
                        [&](int a, int b) { return dist[a] < dist[b]; }
                );

                for (int t = 0; t < k; ++t) {
                    const int j = indices[t];

                    if (compute_self && exclude_self_ && i == j)
                        continue;

                    if (compute_self && i >= j)
                        continue;

                    const ScalarF value = dist[j];
                    triplets.emplace_back(i, j, value);

                    if (compute_self && !triangular_only_) {
                        triplets.emplace_back(j, i, value);
                    }
                }
            }

            Sparse D(n, m);
            D.setFromTriplets(triplets.begin(), triplets.end());
            D.makeCompressed();
            return D;
        }

    private:
        int effectiveK(int d) const {
            const int k =
                    (k_fixed_ > 0)
                    ? k_fixed_
                    : static_cast<int>(std::ceil(k_per_dim_ * double(d)));
            return std::max(1, k);
        }

    private:
        DistanceMetric metric_ = DistanceMetric::L2Distance;
        int k_fixed_ = 0;
        double k_per_dim_ = 2.0;

        bool triangular_only_ = false;
        bool exclude_self_ = false;
    };



// ============================================================
// NanoflannAnnDistance (fast kNN using nanoflann KD-tree)
// Notes:
// - nanoflann KD-tree is typically exact for low dimensions; we still expose it as ANN backend.
// - For cosine: we normalize Y and X and then use L2^2 relation to cosine distance.
//   Here we output cosine distance = 1 - cos = 0.5 * ||x - y||^2 for normalized vectors.
// ============================================================
    namespace detail {
// Row-wise adaptor: each row is a point.
        template <class Mat>
        struct EigenRowAdaptor {
            using Scalar = typename Mat::Scalar;
            const Mat& mat;

            explicit EigenRowAdaptor(const Mat& m) : mat(m) {}

            inline size_t kdtree_get_point_count() const { return static_cast<size_t>(mat.rows()); }

            inline Scalar kdtree_get_pt(const size_t idx, const size_t dim) const {
                return mat(static_cast<Eigen::Index>(idx), static_cast<Eigen::Index>(dim));
            }

            template <class BBOX>
            bool kdtree_get_bbox(BBOX&) const { return false; }
        };

    } // namespace detail

    template <class Scalar>
    class NanoflannAnnDistance: public BaseKnnDistance<Scalar>  {
    public:
        using ScalarF   = typename BaseKnnDistance<Scalar>::ScalarF;
        using Dense     = typename BaseKnnDistance<Scalar>::Dense;
        using DenseF    = typename BaseKnnDistance<Scalar>::DenseF;
        using DenseCRef = typename BaseKnnDistance<Scalar>::DenseCRef;
        using Sparse    = typename BaseKnnDistance<Scalar>::Sparse;

        // Bring base class compute() into scope
        using BaseKnnDistance<Scalar>::compute;

        // ---------------- configuration ----------------
        NanoflannAnnDistance& setMetric(DistanceMetric metric) {
            metric_ = metric;
            return *this;
        }

        NanoflannAnnDistance& setKFixed(int k_fixed) {
            k_fixed_ = k_fixed;
            return *this;
        }

        NanoflannAnnDistance& setKPerDim(double k_per_dim) {
            k_per_dim_ = k_per_dim;
            return *this;
        }

        NanoflannAnnDistance& setTriangularOnly(bool triangular_only) {
            triangular_only_ = triangular_only;
            return *this;
        }

        NanoflannAnnDistance& setExcludeSelf(bool exclude_self) {
            exclude_self_ = exclude_self;
            return *this;
        }

        // Kept for API symmetry; KD-tree knnSearch is typically exact (checks may be unused depending on nanoflann version).
        NanoflannAnnDistance& setChecks(int checks) {
            checks_ = checks;
            return *this;
        }

        // ---------------- API ----------------
        void build(DenseCRef y) {
            if (y.rows() <= 0 || y.cols() <= 0)
                throw std::invalid_argument("NanoflannAnnDistance::build: empty Y");

            this->y_ = y;

            if (metric_ == DistanceMetric::ChordalDistance) {
                detail::normalizeRowsInplace(this->y_);
            }

            adaptor_ = std::make_unique<Adaptor>(this->y_);
            index_   = std::make_unique<Index>(
                    static_cast<int>(this->y_.cols()),
                    *adaptor_,
                    nanoflann::KDTreeSingleIndexAdaptorParams(10)
            );
            index_->buildIndex();
        }


        Sparse compute(DenseCRef x, bool compute_self = false) const override {
            if (!index_)
                throw std::runtime_error("NanoflannAnnDistance::compute: call build(Y) first.");

            if (compute_self && x.rows() != this->y_.rows())
                throw std::invalid_argument("compute_self requires X and Y to have same number of rows");

            if (x.cols() != this->y_.cols())
                throw std::invalid_argument("NanoflannAnnDistance::compute: X.cols != Y.cols");


            const int n = static_cast<int>(x.rows());
            const int m = static_cast<int>(this->y_.rows());
            const int d = static_cast<int>(x.cols());
            const int k = std::min(detail::effectiveK(k_fixed_, k_per_dim_, d), m);

            DenseF x_work = x;
            if (metric_ == DistanceMetric::ChordalDistance) {
                detail::normalizeRowsInplace(x_work);
            }

            std::vector<Eigen::Triplet<ScalarF>> triplets;
            triplets.reserve(static_cast<size_t>(n) * static_cast<size_t>(k));

            using index_t = typename Index::IndexType;
            std::vector<index_t> nn_index(static_cast<size_t>(k));
            std::vector<ScalarF>  nn_dist2(static_cast<size_t>(k));

            for (int i = 0; i < n; ++i) {
                const ScalarF* query = nullptr;
                std::vector<ScalarF> query_vec; // Keep in outer scope
                
                // Check if matrix storage is row-major
                if (x_work.IsRowMajor) {
                    // Row-major: row(i).data() is contiguous
                    query = x_work.row(i).data();
                } else {
                    // Column-major: need to copy row to contiguous buffer
                    query_vec.resize(d);
                    for (int dim = 0; dim < d; ++dim) {
                        query_vec[dim] = x_work(i, dim);
                    }
                    query = query_vec.data();
                }

                const size_t found = index_->knnSearch(
                        query,
                        static_cast<size_t>(k),
                        nn_index.data(),
                        nn_dist2.data()
                );

                for (size_t t = 0; t < found; ++t) {
                    const int j = static_cast<int>(nn_index[t]);

                    if (compute_self && exclude_self_ && i == j)
                        continue;

                    // Skip if i >= j (process only upper triangle)
                    // This matches KnnDistance implementation
                    if (compute_self && i >= j)
                        continue;

                    ScalarF value = std::max<Scalar>(Scalar(0), nn_dist2[t]);
                    if (metric_ == DistanceMetric::L2Distance) {
                        value = std::sqrt(value);
                    } else if (metric_ == DistanceMetric::ChordalDistance) {
                        // cosine distance = 1 - cos = 0.5 * ||x-y||^2 for normalized vectors
                        value *= ScalarF(0.5);
//                        if (metric_ == DistanceMetric::CosineSimilarity) {
//                            value = ScalarF(1) - value;
//                        }
                    }

                    triplets.emplace_back(i, j, value);

                    if (compute_self && !triangular_only_) {
                        triplets.emplace_back(j, i, value);
                    }
                }
            }

            Sparse D(n, m);
            D.setFromTriplets(triplets.begin(), triplets.end());
            D.makeCompressed();
            return D;
        }

    private:
        DistanceMetric metric_ = DistanceMetric::L2Distance;
        int k_fixed_ = 0;
        double k_per_dim_ = 2.0;

        bool triangular_only_ = false;
        bool exclude_self_ = false;

        int checks_ = 64;


        using Adaptor = detail::EigenRowAdaptor<DenseF>;
        using Dist    = nanoflann::L2_Simple_Adaptor<ScalarF, Adaptor>;
        using Index   = nanoflann::KDTreeSingleIndexAdaptor<Dist, Adaptor, -1>;

        std::unique_ptr<Adaptor> adaptor_;
        std::unique_ptr<Index> index_;
    };


} // namespace gsp


#endif //LIBGSP_DISTANCE_H
