//
// Created by mohammad on 2/5/26.
//

#ifndef LIBGSP_DISTANCE_H
#define LIBGSP_DISTANCE_H
#pragma once

#include "libgsp/utils/Types.h"

#include <Eigen/Dense>
#include <stdexcept>

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
    auto pairwiseDistance(
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
    auto pairwiseDistance(
            const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X,
            const Eigen::Matrix<ScalarY, RY, CY, OptY, MRY, MCY>& Y
    )
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RY, OptX, MRX, MRY>
    {
        using OutScalar = gsp::types::float_of<ScalarX>;
        Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic> I =
                Eigen::Matrix<OutScalar, Eigen::Dynamic, Eigen::Dynamic>::Identity(X.cols(), X.cols());
        return pairwiseDistance(X, Y, I);
    }

    template <class ScalarX, int RX, int CX, int OptX, int MRX, int MCX>
    auto pairwiseDistance(const Eigen::Matrix<ScalarX, RX, CX, OptX, MRX, MCX>& X)
    -> Eigen::Matrix<gsp::types::float_of<ScalarX>, RX, RX, OptX, MRX, MRX>
    {
        return pairwiseDistance(X, X);
    }

} // namespace gsp






#endif //LIBGSP_DISTANCE_H
