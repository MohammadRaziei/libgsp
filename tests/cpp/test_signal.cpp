#include <gtest/gtest.h>
#include <vector>
#include <optional>

#include "libgsp/Signal.h"
// tests/test_signal.cpp
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <optional>

#include "libgsp/Signal.h"  // مسیر صحیح هدر خودت

using gsp::Signal;
using gsp::SignalMask;

// ---------- Helpers ----------

// Non-square 3x4 matrix: rows 0/1 do NOT touch col2; row 2 DOES touch col2.
template <typename Scalar>
static Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MakeRectDense() {
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> M(3, 4);
    // row0: uses col0,col1 → both valid in our test
    // row1: uses col1,col3 → both valid
    // row2: uses col0,col2 → touches false column (2)
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

template <typename Scalar>
static Signal<Scalar> MakeSignal_4_with_mask_1101() {
    // values: [10, 20, 30, 40], mask: [1,1,0,1]
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> x(4);
    x << Scalar(10), Scalar(20), Scalar(30), Scalar(40);
    SignalMask mask(4, { {0,true}, {1,true}, {2,false}, {3,true} });
    return Signal<Scalar>(x, mask);
}

// Count of true entries in a mask
static uint32_t CountTrue(const SignalMask& m) {
    uint32_t c = 0;
    for (uint32_t i = 0; i < m.size(); ++i) if (m.at(i)) ++c;
    return c;
}

// ---------- Typed fixture ----------
template <typename Scalar>
class SignalTypedTest : public ::testing::Test {};
using ScalarTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(SignalTypedTest, ScalarTypes);

// ---------- Tests ----------

// Construction and basic access with initializer lists
TYPED_TEST(SignalTypedTest, ConstructFromVectorAndMask) {
    using S = TypeParam;
    Signal<S> sig({S(1), S(2), S(3), S(4)}, { {0,true}, {1,false}, {2,true}, {3,true} });

    ASSERT_EQ(sig.size(), 4);
    EXPECT_TRUE(sig.mask(0));
    EXPECT_FALSE(sig.mask(1));
    EXPECT_TRUE(sig.mask(2));
    EXPECT_TRUE(sig.mask(3));

    // get() respects mask
    EXPECT_TRUE(sig.get(0).has_value());
    EXPECT_FALSE(sig.get(1).has_value());
    EXPECT_TRUE(sig.get(2).has_value());
    EXPECT_TRUE(sig.get(3).has_value());
}

// Matrix multiply (dense): mask propagation cols->rows + applyMask zeros invalid outputs
TYPED_TEST(SignalTypedTest, DenseMul_PropagatesMask_AndZerosMaskedOutputs) {
    using S = TypeParam;
    auto sig = MakeSignal_4_with_mask_1101<S>();
    auto M = MakeRectDense<S>(); // 3x4

    auto out = sig.mul(M);

    ASSERT_EQ(out.size(), 3);
    // Expected mask: [true, true, false] because row2 touches col2 (false)
    EXPECT_TRUE(out.mask(0));
    EXPECT_TRUE(out.mask(1));
    EXPECT_FALSE(out.mask(2));

    // Numeric result for valid rows:
    // row0: 1*10 + 2*20 = 50
    // row1: 1*20 + 1*40 = 60
    // row2: touches false → masked and zeroed by applyMask()
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 50.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 60.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0); // masked
}

// Matrix multiply (sparse) same expectations
TYPED_TEST(SignalTypedTest, SparseMul_PropagatesMask_AndZerosMaskedOutputs) {
    using S = TypeParam;
    auto sig = MakeSignal_4_with_mask_1101<S>();
    auto Ms = MakeRectSparse<S>(); // 3x4

    auto out = sig.mul(Ms);

    ASSERT_EQ(out.size(), 3);
    EXPECT_TRUE(out.mask(0));
    EXPECT_TRUE(out.mask(1));
    EXPECT_FALSE(out.mask(2));

    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 50.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 60.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0);
}

// imul (dense) in-place: should match mul
TYPED_TEST(SignalTypedTest, DenseImul_InPlaceMatchesMul) {
    using S = TypeParam;
    auto sig1 = MakeSignal_4_with_mask_1101<S>();
    auto sig2 = sig1;

    auto M = MakeRectDense<S>(); // 3x4
    auto out = sig1.mul(M);

    sig2.imul(M);

    ASSERT_EQ(sig2.size(), out.size());
    for (int i = 0; i < out.size(); ++i) {
        EXPECT_EQ(sig2.mask(i), out.mask(i));
        EXPECT_DOUBLE_EQ(static_cast<double>(sig2.signal(i)),
                         static_cast<double>(out.signal(i)));
    }
}

// apply: only valid entries are transformed; invalid entries remain zero in numeric & keep mask
TYPED_TEST(SignalTypedTest, Apply_OnlyOnValidIndices) {
    using S = TypeParam;
    auto sig = MakeSignal_4_with_mask_1101<S>(); // [10,20,30,40], mask [1,1,0,1]

    auto out = sig.apply([](const S& v){ return v*v; });

    ASSERT_EQ(out.size(), 4);
    // Mask unchanged
    EXPECT_TRUE(out.mask(0));
    EXPECT_TRUE(out.mask(1));
    EXPECT_FALSE(out.mask(2));
    EXPECT_TRUE(out.mask(3));

    // Values squared on valid indices; invalid index left default-initialized (0)
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 100.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 400.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(3)), 1600.0);
}

// applyInplace: same semantics as apply, but in-place
TYPED_TEST(SignalTypedTest, ApplyInplace_OnlyOnValidIndices) {
    using S = TypeParam;
    auto sig = MakeSignal_4_with_mask_1101<S>();
    sig.applyInplace([](const S& v){ return v + S(1); });

    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(0)), 11.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(1)), 21.0);
    // masked index 2 must remain zero
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(2)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(3)), 41.0);
}

// Elementwise operators: mask union + applyMask
TYPED_TEST(SignalTypedTest, ElementwiseOps_CombineMasksAndZeroMasked) {
    using S = TypeParam;

    // sigA: values [1,2,3,4], mask [1,0,1,1]
    Eigen::Matrix<S, Eigen::Dynamic, 1> a(4);
    a << S(1), S(2), S(3), S(4);
    SignalMask maskA(4, { {0,true}, {1,false}, {2,true}, {3,true} });
    Signal<S> sigA(a, maskA);
    sigA.applyMask();

    // sigB: values [10,20,30,40], mask [1,1,0,1]
    auto sigB = MakeSignal_4_with_mask_1101<S>().applyMask();

    // A + B
    auto ssum = sigA + sigB;
    // Combined mask = union of falses: indices 1 and 2 false
    EXPECT_TRUE(ssum.mask(0));
    EXPECT_FALSE(ssum.mask(1));
    EXPECT_FALSE(ssum.mask(2));
    EXPECT_TRUE(ssum.mask(3));
    // Values where valid: idx0 → 1+10=11, idx3 → 4+40=44
    EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(0)), 11.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(1)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(2)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(3)), 44.0);

    // A * B
    auto sprod = sigA * sigB;
    EXPECT_TRUE(sprod.mask(0));
    EXPECT_FALSE(sprod.mask(1));
    EXPECT_FALSE(sprod.mask(2));
    EXPECT_TRUE(sprod.mask(3));
    EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(0)), 10.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(1)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(2)), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(3)), 160.0);
}

// set(idx, optional): setting a value should make index valid; nullopt should invalidate
TYPED_TEST(SignalTypedTest, SetOptional_TogglesMaskSemantics) {
    using S = TypeParam;
    Signal<S> sig(4); // all zeros, mask true by default

    // Invalidate idx 2
    sig.setMask(2, false);
    EXPECT_FALSE(sig.mask(2));
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(2)), 0.0);

    // Set a value at idx 2 → EXPECTED: becomes valid (mask true)
    sig.set(2, std::optional<S>(S(5)));
    EXPECT_TRUE(sig.mask(2)) << "Setting a value should mark index as valid (mask true)";
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(2)), 5.0);

    // Set nullopt → invalid and zero
    sig.set(2, std::optional<S>());
    EXPECT_FALSE(sig.mask(2));
    EXPECT_DOUBLE_EQ(static_cast<double>(sig.signal(2)), 0.0);
}

// compressed(): should keep only valid entries in order, size == count(valid)
TYPED_TEST(SignalTypedTest, Compressed_ContainsOnlyValidInOrder) {
    using S = TypeParam;

    // values [1,2,3,4], mask [1,0,1,1] → valid indices {0,2,3}
    Eigen::Matrix<S, Eigen::Dynamic, 1> a(4);
    a << S(1), S(2), S(3), S(4);
    SignalMask mask(4, { {0,true}, {1,false}, {2,true}, {3,true} });
    Signal<S> sig(a, mask);
    sig.applyMask();

    auto comp = sig.compressed();
    const uint32_t expected_size = CountTrue(mask);
    ASSERT_EQ(static_cast<uint32_t>(comp.size()), expected_size)
        << "compressed() size must equal #valid entries";

    // Expect values [1,3,4]
    ASSERT_EQ(comp.size(), 3);
    EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(0)), 1.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(1)), 3.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(2)), 4.0);
    EXPECT_TRUE(comp.mask(0));
    EXPECT_TRUE(comp.mask(1));
    EXPECT_TRUE(comp.mask(2));
}

