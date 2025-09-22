// tests/test_signal.cpp
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <optional>
#include "libgsp/Signal.h"

using gsp::Signal;
using gsp::SignalMask;

// ---- Typed fixture holding shared state for all tests ----
template <typename S>
class SignalFixture : public ::testing::Test {
protected:
  using Vec = Eigen::Matrix<S, Eigen::Dynamic, 1>;
  using Mat = Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic>;
  using SpMat = Eigen::SparseMatrix<S>;

  // Shared objects
  Mat  M_rect;     // 3x4, rows 0/1 don't touch col2, row2 touches col2
  SpMat Ms_rect;   // sparse view of M_rect
  Signal<S> sig;   // values [10,20,30,40], mask [1,1,0,1]

  void SetUp() override {
    // Build M_rect (3x4)
    M_rect.resize(3,4);
    M_rect <<
      S(1), S(2), S(0), S(0),
      S(0), S(1), S(0), S(1),
      S(5), S(0), S(7), S(0);
    Ms_rect = M_rect.sparseView();

    // Build signal with mask [1,1,0,1]
    Vec x(4);
    x << S(10), S(20), S(30), S(40);
    SignalMask mask(4, { {0,true}, {1,true}, {2,false}, {3,true} });
    sig = Signal<S>(x, mask); // applyMask() را هرجا نیاز شد می‌زنیم
  }

  static uint32_t countTrue(const SignalMask& m) {
    uint32_t c = 0;
    for (uint32_t i = 0; i < m.size(); ++i) if (m.at(i)) ++c;
    return c;
  }
};

using ScalarTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(SignalFixture, ScalarTypes);

// ---- Tests ----

TYPED_TEST(SignalFixture, ConstructFromVectorAndMask) {
  using S = TypeParam;
  Signal<S> s2({S(1), S(2), S(3), S(4)}, { {0,true}, {1,false}, {2,true}, {3,true} });
  ASSERT_EQ(s2.size(), 4);
  EXPECT_TRUE (s2.mask(0));
  EXPECT_FALSE(s2.mask(1));
  EXPECT_TRUE (s2.mask(2));
  EXPECT_TRUE (s2.mask(3));
  EXPECT_TRUE (s2.get(0).has_value());
  EXPECT_FALSE(s2.get(1).has_value());
}

TYPED_TEST(SignalFixture, DenseMul_PropagatesMask_AndZerosMaskedOutputs) {
  using S = TypeParam;
  auto out = this->sig.mul(this->M_rect); // 3x4 * 4x1 -> 3x1, mask cols->rows

  ASSERT_EQ(out.size(), 3);
  EXPECT_TRUE (out.mask(0));
  EXPECT_TRUE (out.mask(1));
  EXPECT_FALSE(out.mask(2)); // row2 touches col2(false)

  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 50.0); // 1*10 + 2*20
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 60.0); // 1*20 + 1*40
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0);  // masked by applyMask()
}

TYPED_TEST(SignalFixture, SparseMul_PropagatesMask_AndZerosMaskedOutputs) {
  using S = TypeParam;
  auto out = this->sig.mul(this->Ms_rect);

  ASSERT_EQ(out.size(), 3);
  EXPECT_TRUE (out.mask(0));
  EXPECT_TRUE (out.mask(1));
  EXPECT_FALSE(out.mask(2));

  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 50.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 60.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0);
}

TYPED_TEST(SignalFixture, DenseImul_InPlaceMatchesMul) {
  using S = TypeParam;
  auto s_copy = this->sig;
  auto out    = this->sig.mul(this->M_rect);
  s_copy.imul(this->M_rect);

  ASSERT_EQ(s_copy.size(), out.size());
  for (int i = 0; i < out.size(); ++i) {
    EXPECT_EQ(s_copy.mask(i), out.mask(i));
    EXPECT_DOUBLE_EQ(static_cast<double>(s_copy.signal(i)),
                     static_cast<double>(out.signal(i)));
  }
}

TYPED_TEST(SignalFixture, Apply_OnlyOnValidIndices) {
  using S = TypeParam;
  auto out = this->sig.apply([](const S& v){ return v*v; });

  ASSERT_EQ(out.size(), 4);
  EXPECT_TRUE (out.mask(0));
  EXPECT_TRUE (out.mask(1));
  EXPECT_FALSE(out.mask(2));
  EXPECT_TRUE (out.mask(3));

  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(0)), 100.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(1)), 400.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(2)), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(out.signal(3)), 1600.0);
}

TYPED_TEST(SignalFixture, ApplyInplace_OnlyOnValidIndices) {
  using S = TypeParam;
  auto s = this->sig;
  s.applyInplace([](const S& v){ return v + S(1); });

  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(0)), 11.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(1)), 21.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(2)), 0.0); // masked stays zero
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(3)), 41.0);
}

TYPED_TEST(SignalFixture, ElementwiseOps_CombineMasksAndZeroMasked) {
  using S = TypeParam;

  // sigA: [1,2,3,4], mask [1,0,1,1]
  Eigen::Matrix<S, Eigen::Dynamic, 1> a(4);
  a << S(1), S(2), S(3), S(4);
  SignalMask maskA(4, { {0,true}, {1,false}, {2,true}, {3,true} });
  Signal<S> sigA(a, maskA);
  sigA.applyMask();

  // sigB: fixture’s signal (mask [1,1,0,1])
  auto sigB = this->sig;
  sigB.applyMask();

  auto ssum  = sigA + sigB;
  auto sprod = sigA * sigB;

  // Mask union: idx 1 and 2 false
  for (const auto* s : {&ssum, &sprod}) {
    EXPECT_TRUE (s->mask(0));
    EXPECT_FALSE(s->mask(1));
    EXPECT_FALSE(s->mask(2));
    EXPECT_TRUE (s->mask(3));
  }

  EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(0)), 11.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(1)), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(2)), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(ssum.signal(3)), 44.0);

  EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(0)), 10.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(1)), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(2)), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(sprod.signal(3)), 160.0);
}

TYPED_TEST(SignalFixture, SetOptional_TogglesMaskSemantics) {
  using S = TypeParam;
  Signal<S> s(4); // all zeros, mask true by default

  s.setMask(2, false);
  s.applyMask();
  EXPECT_FALSE(s.mask(2));
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(2)), 0.0);

  s.set(2, std::optional<S>(S(5)));
  EXPECT_TRUE(s.mask(2)) << "Setting a value should mark index as valid";
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(2)), 5.0);

  s.set(2, std::optional<S>());
  EXPECT_FALSE(s.mask(2));
  EXPECT_DOUBLE_EQ(static_cast<double>(s.signal(2)), 0.0);
}

TYPED_TEST(SignalFixture, Compressed_ContainsOnlyValidInOrder) {
  using S = TypeParam;

  Eigen::Matrix<S, Eigen::Dynamic, 1> a(4);
  a << S(1), S(2), S(3), S(4);
  SignalMask mask(4, { {0,true}, {1,false}, {2,true}, {3,true} });
  Signal<S> s(a, mask);
  s.applyMask();

  auto comp = s.compressed();
  const uint32_t expected = this->countTrue(mask);

  ASSERT_EQ(static_cast<uint32_t>(comp.size()), expected);
  ASSERT_EQ(comp.size(), 3);
  EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(0)), 1.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(1)), 3.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(comp.signal(2)), 4.0);
  EXPECT_TRUE (comp.mask(0));
  EXPECT_TRUE (comp.mask(1));
  EXPECT_TRUE (comp.mask(2));
}
