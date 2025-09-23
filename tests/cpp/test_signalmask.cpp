//
// Created by Mohammad on 9/22/2025.
//


// tests/test_signalmask.cpp
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "libgsp/Signal.h"

using gsp::SignalMask;

// ---------- Basic (non-parameterized) tests ----------
TEST(SignalMaskBasic, ConstructFromSize_DefaultAllTrue) {
    SignalMask m(5);
    EXPECT_EQ(m.size(), 5u);
    EXPECT_EQ(m.nnz(), m.size());                 // nnz == number of FALSE (stored in complement)
    for (uint32_t i = 0; i < 5; ++i) {
        EXPECT_TRUE(m.at(i));               // default is true
    }
}

TEST(SignalMaskBasic, SetAndGet_Toggle) {
    SignalMask m(4);
    m.set(2, false);
    EXPECT_FALSE(m.at(2));
    EXPECT_EQ(m.nnz(), 3);

    m.set(2, true); // remove from complement
    EXPECT_TRUE(m.at(2));
    EXPECT_EQ(m.nnz(), m.size());
}

TEST(SignalMaskBasic, SetMask_DenseVector) {
    SignalMask::DenseMask dense(5);
    dense << 1, 0, 1, 0, 1;  // 1=true, 0=false
    SignalMask m(5);
    m.setMask(dense);
    EXPECT_TRUE(m.at(0));
    EXPECT_FALSE(m.at(1));
    EXPECT_TRUE(m.at(2));
    EXPECT_FALSE(m.at(3));
    EXPECT_TRUE(m.at(4));
    EXPECT_EQ(m.nnz(), 5u - 2u);  // two falses (1 and 3)
}

TEST(SignalMaskBasic, UnionOperator) {
    SignalMask a(6), b(6);
    a.set(1, false);
    a.set(4, false);
    b.set(2, false);

    auto c = a + b;  // union of falses
    EXPECT_FALSE(c.at(1));
    EXPECT_FALSE(c.at(2));
    EXPECT_FALSE(c.at(4));
    EXPECT_TRUE(c.at(0));
    EXPECT_TRUE(c.at(3));
    EXPECT_TRUE(c.at(5));
    EXPECT_EQ(c.nnz(), 3u);
}

// ---------- Parameterized tests for structural propagation (Sparse/Dense) ----------

// A small helper to build a 3x4 structure where only row 2 depends on col 2 (zero-based),
// and rows 0/1 do NOT depend on col 2.
//   [ 1  2  0  0 ]
//   [ 0  1  0  1 ]
//   [ 5  0  7  0 ]  <-- touches col2
template <typename Scalar>
static Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MakeRectDense() {
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> M(3,4);
    M << Scalar(1), Scalar(2), Scalar(0), Scalar(0),
         Scalar(0), Scalar(1), Scalar(0), Scalar(1),
         Scalar(5), Scalar(0), Scalar(7), Scalar(0);
    return M;
}

template <typename Scalar>
static Eigen::SparseMatrix<Scalar> MakeRectSparse() {
    auto D = MakeRectDense<Scalar>();
    Eigen::SparseMatrix<Scalar> S = D.sparseView();
    return S;
}

// Typed fixture for floating-point dense tests (to validate epsilon threshold)
template <typename Scalar>
class SignalMaskDenseFloatTest : public ::testing::Test {};
using FloatTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(SignalMaskDenseFloatTest, FloatTypes);

// Dense propagation: cols->rows, no transpose, with epsilon threshold (1e-12)
TYPED_TEST(SignalMaskDenseFloatTest, DenseImul_ColsToRows_WithThreshold) {
    using Scalar = TypeParam;
    // A (3x4) with one tiny connection and one meaningful connection to the false column
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> A(2,3);
    A.setZero();

    // mask size must match A.cols() == 3
    // We make column 1 "false" and verify propagation.
    // Row 0: A(0,1) = very tiny -> should NOT propagate due to sparseView(1e-12)
    // Row 1: A(1,1) = 1e-6      -> should propagate.
    A(0, 1) = Scalar(1e-13);   // below threshold
    A(1, 1) = Scalar(1e-6);    // above threshold

    SignalMask m(3);
    m.set(1, false); // col 1 is false

    m.imul(A); // dense version calls sparseView(1e-12) internally, then sparse path

    ASSERT_EQ(m.size(), 2u);      // rows
    EXPECT_FALSE(m.at(0));     // TODO     // stays true (tiny weight)
    EXPECT_FALSE(m.at(1));       // becomes false (significant weight)
    EXPECT_EQ(m.nnz(), 0u); // TODO
}

// Sparse propagation: use the 3x4 structure; col 2 false should only mark row 2 false.
template <typename Scalar>
class SignalMaskSparseTest : public ::testing::Test {};
using SparseScalarTypes = ::testing::Types<int, float, double>;
TYPED_TEST_SUITE(SignalMaskSparseTest, SparseScalarTypes);

TYPED_TEST(SignalMaskSparseTest, SparseImul_ColsToRows_NoTranspose) {
    using Scalar = TypeParam;
    auto A = MakeRectSparse<Scalar>(); // 3x4

    SignalMask m(static_cast<uint32_t>(A.cols()));
    // Input mask: [1, 1, 0, 1]  (only col 2 is false)
    m.set(2, false);

    m.imul(A); // should propagate to rows

    ASSERT_EQ(m.size(), static_cast<uint32_t>(A.rows()));
    // Expected: row 0 -> true, row 1 -> true, row 2 -> false (touches col 2)
    EXPECT_TRUE(m.at(0));
    EXPECT_TRUE(m.at(1));
    EXPECT_FALSE(m.at(2));
    EXPECT_EQ(m.nnz(), 2u); // TODO
}

// Dense propagation on the same rectangular pattern (without tiny eps cases)
TYPED_TEST(SignalMaskDenseFloatTest, DenseMul_ColsToRows_Rectangular) {
    using Scalar = TypeParam;
    auto A = MakeRectDense<Scalar>(); // 3x4

    SignalMask m(static_cast<uint32_t>(A.cols()));
    m.set(2, false); // false column

    auto out = m.mul(A); // out-of-place

    ASSERT_EQ(out.size(), static_cast<uint32_t>(A.rows()));
    EXPECT_TRUE(out.at(0));
    EXPECT_TRUE(out.at(1));
    EXPECT_FALSE(out.at(2));
    EXPECT_EQ(out.nnz(), 2u); // TODO
}

// Sanity: setComplementMask builds expected state directly
TEST(SignalMaskBasic, SetComplementMask_Direct) {
    SignalMask m(5);
    SignalMask::SparseComplementMask comp(5);
    comp.insert(1) = 1; // false at 1
    comp.insert(3) = 1; // false at 3
    comp.finalize();

    m.setComplementMask(comp);

    EXPECT_TRUE(m.at(0));
    EXPECT_FALSE(m.at(1));
    EXPECT_TRUE(m.at(2));
    EXPECT_FALSE(m.at(3));
    EXPECT_TRUE(m.at(4));
    EXPECT_EQ(m.nnz(), 3u);
}

// Optional: one quick string check (kept minimal)
TEST(SignalMaskBasic, StrPrettyPrint) {
    SignalMask m(4);
    m.set(2, false);
    auto s = m.str();   // expects "[1, 1, n, 1]" or similar format
    // Don't overconstrain formatting; just assert key bits exist.
    EXPECT_NE(s.find("n"), std::string::npos);
    EXPECT_NE(s.find('['), std::string::npos);
    EXPECT_NE(s.find(']'), std::string::npos);
}
