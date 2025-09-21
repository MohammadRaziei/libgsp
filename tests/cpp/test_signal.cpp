#include <gtest/gtest.h>
#include <vector>
#include <optional>

#include "libgsp/Signal.h"

using namespace gsp;

// Fixture for common test setup
class SignalTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
        vec3_ = Eigen::VectorXd(3);
        vec3_ << 1.0, 2.0, 3.0;
        
        vec5_ = Eigen::VectorXd(5);
        vec5_ << 1.0, 2.0, 3.0, 4.0, 5.0;
    }

    Eigen::VectorXd vec3_;
    Eigen::VectorXd vec5_;
};

// ---------- Constructor Tests ----------
TEST_F(SignalTest, DefaultConstructorCreatesEmptySignal) {
    Signal<double> s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.signal().isZero(0));
}

TEST_F(SignalTest, SizeConstructorCreatesZeroedSignalWithValidMask) {
    const int size = 5;
    Signal<double> s(size);
    
    EXPECT_EQ(s.size(), size);
    EXPECT_TRUE(s.signal().isZero());
    for (int i = 0; i < size; ++i) {
        EXPECT_TRUE(s.mask().at(i));
    }
}

TEST_F(SignalTest, VectorConstructorPreservesValuesAndCreatesValidMask) {
    Signal<double> s(vec3_);
    
    EXPECT_EQ(s.size(), 3);
    for (int i = 0; i < 3; ++i) {
        EXPECT_DOUBLE_EQ(s.signal()(i), vec3_(i));
        EXPECT_TRUE(s.mask().at(i));
    }
}

TEST_F(SignalTest, InitializerListConstructorWorksCorrectly) {
    Signal<double> s{1.5, 2.5, 3.5};
    
    EXPECT_EQ(s.size(), 3);
    EXPECT_DOUBLE_EQ(s.signal()(0), 1.5);
    EXPECT_DOUBLE_EQ(s.signal()(1), 2.5);
    EXPECT_DOUBLE_EQ(s.signal()(2), 3.5);
    
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(s.mask().at(i));
    }
}

// ---------- Mask Integration Tests ----------
TEST_F(SignalTest, ConstructorWithCustomMaskPreservesMaskState) {
    SignalMask mask(3);
    mask.set(1, false); // Mask out middle element
    
    Signal<double> s(vec3_, mask);
    
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1));
    EXPECT_TRUE(s.mask().at(2));
    
    // Values should still be preserved
    for (int i = 0; i < 3; ++i) {
        EXPECT_DOUBLE_EQ(s.signal()(i), vec3_(i));
    }
}

TEST_F(SignalTest, OptionalVectorConstructorHandlesNulloptCorrectly) {
    std::vector<std::optional<double>> opts = {
        1.0, std::nullopt, 3.0, std::nullopt, 5.0
    };
    
    Signal<double> s(opts);
    
    EXPECT_EQ(s.size(), 5);
    
    // Check values
    EXPECT_DOUBLE_EQ(s.signal()(0), 1.0);
    EXPECT_DOUBLE_EQ(s.signal()(1), 0.0); // nullopt becomes 0.0
    EXPECT_DOUBLE_EQ(s.signal()(2), 3.0);
    EXPECT_DOUBLE_EQ(s.signal()(3), 0.0); // nullopt becomes 0.0
    EXPECT_DOUBLE_EQ(s.signal()(4), 5.0);
    
    // Check mask
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1)); // nullopt becomes masked
    EXPECT_TRUE(s.mask().at(2));
    EXPECT_FALSE(s.mask().at(3)); // nullopt becomes masked
    EXPECT_TRUE(s.mask().at(4));
}

// ---------- Mask Manipulation Tests ----------
TEST_F(SignalTest, SetMaskReplacesExistingMask) {
    Signal<double> s(vec3_);
    
    SignalMask new_mask(3);
    new_mask.set(0, false);
    new_mask.set(2, false);
    
    s.setMask(new_mask);
    
    EXPECT_FALSE(s.mask().at(0));
    EXPECT_TRUE(s.mask().at(1));
    EXPECT_FALSE(s.mask().at(2));
    
    // Signal values should remain unchanged
    for (int i = 0; i < 3; ++i) {
        EXPECT_DOUBLE_EQ(s.signal()(i), vec3_(i));
    }
}

TEST_F(SignalTest, SetIndividualMaskElementWorks) {
    Signal<double> s(vec3_);
    
    s.setMask(1, false);
    
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1));
    EXPECT_TRUE(s.mask().at(2));
}

TEST_F(SignalTest, SetComplementMaskWorksWithSparseRepresentation) {
    Signal<double> s(vec3_);
    
    SignalMask::SparseComplementMask sparse_mask(3);
    sparse_mask.insert(1) = 1; // Index 1 is false in complement
    
    s.setComplementMask(sparse_mask);
    
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1)); // This should be false due to complement
    EXPECT_TRUE(s.mask().at(2));
}

// ---------- Element Access Tests ----------
TEST_F(SignalTest, GetReturnsOptionalWithCorrectValueForValidElements) {
    Signal<double> s(vec3_);
    s.setMask(1, false); // Mask out middle element
    
    auto opt0 = s.get(0);
    auto opt1 = s.get(1);
    auto opt2 = s.get(2);
    
    EXPECT_TRUE(opt0.has_value());
    EXPECT_DOUBLE_EQ(*opt0, 1.0);
    
    EXPECT_FALSE(opt1.has_value()); // Masked element
    
    EXPECT_TRUE(opt2.has_value());
    EXPECT_DOUBLE_EQ(*opt2, 3.0);
}

TEST_F(SignalTest, SetWithValueUpdatesValueAndUnmasks) {
    Signal<double> s(3);
    s.setMask(1, false); // Start with masked element
    
    s.set(1, 99.0);
    
    EXPECT_DOUBLE_EQ(s.signal()(1), 99.0);
    EXPECT_TRUE(s.mask().at(1)); // Should be unmasked now
}

TEST_F(SignalTest, SetWithNulloptMasksElementAndSetsToZero) {
    Signal<double> s(vec3_);
    
    s.set(1, std::nullopt);
    
    EXPECT_DOUBLE_EQ(s.signal()(1), 0.0);
    EXPECT_FALSE(s.mask().at(1));
}

// ---------- Arithmetic Operation Tests ----------
TEST_F(SignalTest, AdditionCombinesMasksCorrectly) {
    // s1: [1.0, n, 3.0] (masked at index 1)
    Signal<double> s1({1.0, 2.0, 3.0}, {{1, false}});
    
    // s2: [4.0, 5.0, n] (masked at index 2)  
    Signal<double> s2({4.0, 5.0, 6.0}, {{2, false}});
    
    Signal<double> result = s1 + s2;
    
    // Values should be added
    EXPECT_DOUBLE_EQ(result.signal()(0), 5.0); // 1.0 + 4.0
    EXPECT_DOUBLE_EQ(result.signal()(1), 7.0); // 2.0 + 5.0
    EXPECT_DOUBLE_EQ(result.signal()(2), 9.0); // 3.0 + 6.0
    
    // Mask should be intersection: only index 0 should be valid
    // because index 1 is masked in s1 and index 2 is masked in s2
    EXPECT_TRUE(result.mask().at(0));
    EXPECT_FALSE(result.mask().at(1));
    EXPECT_FALSE(result.mask().at(2));
}

TEST_F(SignalTest, MultiplicationCombinesMasksCorrectly) {
    Signal<double> s1({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    Signal<double> s2({4.0, 5.0, 6.0}, {{2, false}}); // [4.0, 5.0, n]
    
    Signal<double> result = s1 * s2;
    
    EXPECT_DOUBLE_EQ(result.signal()(0), 4.0);  // 1.0 * 4.0
    EXPECT_DOUBLE_EQ(result.signal()(1), 10.0); // 2.0 * 5.0
    EXPECT_DOUBLE_EQ(result.signal()(2), 18.0); // 3.0 * 6.0
    
    // Mask intersection: only index 0 should be valid
    EXPECT_TRUE(result.mask().at(0));
    EXPECT_FALSE(result.mask().at(1));
    EXPECT_FALSE(result.mask().at(2));
}

TEST_F(SignalTest, InPlaceOperationsUpdateBothSignalAndMask) {
    Signal<double> s1({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    Signal<double> s2({4.0, 5.0, 6.0}, {{2, false}}); // [4.0, 5.0, n]
    
    s1 += s2;
    
    EXPECT_DOUBLE_EQ(s1.signal()(0), 5.0);  // 1.0 + 4.0
    EXPECT_DOUBLE_EQ(s1.signal()(1), 7.0);  // 2.0 + 5.0
    EXPECT_DOUBLE_EQ(s1.signal()(2), 9.0);  // 3.0 + 6.0
    
    // Mask should be updated to intersection
    EXPECT_TRUE(s1.mask().at(0));
    EXPECT_FALSE(s1.mask().at(1));
    EXPECT_FALSE(s1.mask().at(2));
}

// ---------- Matrix Multiplication Tests ----------
TEST_F(SignalTest, DenseMatrixMultiplicationPreservesMaskLogic) {
    Signal<double> s({1.0, 2.0, 3.0});
    
    Eigen::MatrixXd M(2, 3);
    M << 1.0, 0.0, 1.0,
         0.0, 1.0, 0.0;
    
    Signal<double> result = s.mul(M);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_DOUBLE_EQ(result.signal()(0), 4.0); // 1*1 + 0*2 + 1*3
    EXPECT_DOUBLE_EQ(result.signal()(1), 2.0); // 0*1 + 1*2 + 0*3
    
    // All inputs were valid, so outputs should be valid
    EXPECT_TRUE(result.mask().at(0));
    EXPECT_TRUE(result.mask().at(1));
}

TEST_F(SignalTest, MatrixMultiplicationWithPartialMask) {
    // s: [1.0, n, 3.0] (masked at index 1)
    Signal<double> s({1.0, 2.0, 3.0}, {{1, false}});
    
    Eigen::MatrixXd M(2, 3);
    M << 1.0, 1.0, 1.0,
         1.0, 1.0, 1.0;
    
    Signal<double> result = s.mul(M);
    
    // Result: M * [1, n, 3] = [1*1 + 1*n + 1*3, 1*1 + 1*n + 1*3] = [4, 4]
    EXPECT_DOUBLE_EQ(result.signal()(0), 4.0);
    EXPECT_DOUBLE_EQ(result.signal()(1), 4.0);
    
    // Both outputs depend on the masked column, so both should be masked out
    EXPECT_FALSE(result.mask().at(0));
    EXPECT_FALSE(result.mask().at(1));
}

TEST_F(SignalTest, SparseMatrixMultiplicationWorks) {
    Signal<double> s({1.0, 2.0, 3.0});
    
    Eigen::SparseMatrix<double> M(2, 3);
    M.insert(0, 0) = 1.0;
    M.insert(0, 2) = 1.0;
    M.insert(1, 1) = 1.0;
    
    Signal<double> result = s.mul(M);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_DOUBLE_EQ(result.signal()(0), 4.0); // 1*1 + 1*3
    EXPECT_DOUBLE_EQ(result.signal()(1), 2.0); // 1*2
    
    EXPECT_TRUE(result.mask().at(0));
    EXPECT_TRUE(result.mask().at(1));
}

// ---------- Function Application Tests ----------
TEST_F(SignalTest, ApplyFunctionPreservesMask) {
    Signal<double> s({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    
    auto squared = s.apply([](const double& x) { return x * x; });
    
    EXPECT_EQ(squared.size(), 3);
    EXPECT_DOUBLE_EQ(squared.signal()(0), 1.0);
    EXPECT_DOUBLE_EQ(squared.signal()(1), 4.0); // But masked out
    EXPECT_DOUBLE_EQ(squared.signal()(2), 9.0);
    
    // Mask should be preserved exactly
    EXPECT_TRUE(squared.mask().at(0));
    EXPECT_FALSE(squared.mask().at(1));
    EXPECT_TRUE(squared.mask().at(2));
}

TEST_F(SignalTest, ApplyInplaceFunctionUpdatesOnlyValidElements) {
    Signal<double> s({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    
    s.applyInplace([](const double& x) { return x + 10.0; });
    
    EXPECT_DOUBLE_EQ(s.signal()(0), 11.0); // 1.0 + 10.0
    EXPECT_DOUBLE_EQ(s.signal()(1), 2.0);  // Unchanged (masked out)
    EXPECT_DOUBLE_EQ(s.signal()(2), 13.0); // 3.0 + 10.0
    
    // Mask unchanged
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1));
    EXPECT_TRUE(s.mask().at(2));
}

// ---------- Edge Case Tests ----------
TEST_F(SignalTest, EmptySignalOperations) {
    Signal<double> empty;
    Signal<double> non_empty({1.0, 2.0});
    
    // Should throw or handle gracefully - testing basic behavior
    EXPECT_EQ(empty.size(), 0);
    EXPECT_EQ(non_empty.size(), 2);
}

TEST_F(SignalTest, ApplyMaskZerosOutMaskedElements) {
    Signal<double> s({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    
    s.applyMask();
    
    EXPECT_DOUBLE_EQ(s.signal()(0), 1.0);
    EXPECT_DOUBLE_EQ(s.signal()(1), 0.0); // Masked element zeroed out
    EXPECT_DOUBLE_EQ(s.signal()(2), 3.0);
    
    // Mask unchanged
    EXPECT_TRUE(s.mask().at(0));
    EXPECT_FALSE(s.mask().at(1));
    EXPECT_TRUE(s.mask().at(2));
}

TEST_F(SignalTest, CompressedReturnsOnlyValidElements) {
    Signal<double> s({1.0, 2.0, 3.0, 4.0}, {{1, false}, {3, false}}); // [1.0, n, 3.0, n]
    
    Signal<double> compressed = s.compressed();
    
    EXPECT_EQ(compressed.size(), 2); // Only 2 valid elements
    EXPECT_DOUBLE_EQ(compressed.signal()(0), 1.0);
    EXPECT_DOUBLE_EQ(compressed.signal()(1), 3.0);
    
    // All elements in compressed should be valid
    EXPECT_TRUE(compressed.mask().at(0));
    EXPECT_TRUE(compressed.mask().at(1));
}

// ---------- String Representation Test ----------
TEST_F(SignalTest, StringRepresentationShowsMaskedElements) {
    Signal<double> s({1.0, 2.0, 3.0}, {{1, false}}); // [1.0, n, 3.0]
    
    std::string str = s.str();
    
    // Should contain representation showing masked element
    EXPECT_TRUE(str.find("n") != std::string::npos);
    EXPECT_TRUE(str.find("1") != std::string::npos);
    EXPECT_TRUE(str.find("3") != std::string::npos);
}

// ---------- Type Conversion Tests ----------
TEST_F(SignalTest, DifferentSignalTypesWork) {
    Signal<int> int_signal({1, 2, 3});
    Signal<float> float_signal({1.5f, 2.5f, 3.5f});
    
    EXPECT_EQ(int_signal.size(), 3);
    EXPECT_EQ(float_signal.size(), 3);
    
    EXPECT_EQ(int_signal.signal()(0), 1);
    EXPECT_FLOAT_EQ(float_signal.signal()(0), 1.
