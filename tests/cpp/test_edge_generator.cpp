//
// Created by Mohammad on 1/3/2026.
//

#include <gtest/gtest.h>
#include <vector>
#include <optional>

#include "libgsp/Graph.h"
#include "libgsp/iterators/EdgeGenerator.hpp"

class EdgeGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test data
    }
};

TEST_F(EdgeGeneratorTest, EmptyGraph) {
    gsp::DenseGraph graph(0u);
    
    auto generator = graph.iterEdges();
    auto edge = generator.next();
    
    EXPECT_FALSE(edge.has_value());
}

TEST_F(EdgeGeneratorTest, DenseGraphUndirectedNoEdges) {
    gsp::DenseGraph graph(3);
    
    auto generator = graph.iterEdges();
    auto edge = generator.next();
    
    EXPECT_FALSE(edge.has_value());
}

TEST_F(EdgeGeneratorTest, DenseGraphUndirectedWithEdges) {
    gsp::DenseGraph graph(3);
    
    // Set up a simple adjacency matrix
    Eigen::MatrixXd weights(3, 3);
    weights << 0, 1, 2,
               1, 0, 3,
               2, 3, 0;
    
    graph.setWeights(weights, false); // undirected
    
    auto generator = graph.iterEdges();
    
    // Should get edges: (0,1), (0,2), (1,2) - only upper triangle for undirected
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 1);
    EXPECT_DOUBLE_EQ(edge1->weight, 1.0);
    
    auto edge2 = generator.next();
    ASSERT_TRUE(edge2.has_value());
    EXPECT_EQ(edge2->source, 0);
    EXPECT_EQ(edge2->target, 2);
    EXPECT_DOUBLE_EQ(edge2->weight, 2.0);
    
    auto edge3 = generator.next();
    ASSERT_TRUE(edge3.has_value());
    EXPECT_EQ(edge3->source, 1);
    EXPECT_EQ(edge3->target, 2);
    EXPECT_DOUBLE_EQ(edge3->weight, 3.0);
    
    auto edge4 = generator.next();
    EXPECT_FALSE(edge4.has_value()); // No more edges
}

TEST_F(EdgeGeneratorTest, DenseGraphDirectedWithEdges) {
    gsp::DenseGraph graph(3);
    
    // Set up a simple adjacency matrix
    Eigen::MatrixXd weights(3, 3);
    weights << 0, 1, 2,
               4, 0, 3,
               5, 6, 0;
    
    graph.setWeights(weights, true); // directed
    
    auto generator = graph.iterEdges();
    
    // Should get all non-zero edges: (0,1), (0,2), (1,0), (1,2), (2,0), (2,1)
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 1);
    EXPECT_DOUBLE_EQ(edge1->weight, 1.0);
    
    auto edge2 = generator.next();
    ASSERT_TRUE(edge2.has_value());
    EXPECT_EQ(edge2->source, 0);
    EXPECT_EQ(edge2->target, 2);
    EXPECT_DOUBLE_EQ(edge2->weight, 2.0);
    
    auto edge3 = generator.next();
    ASSERT_TRUE(edge3.has_value());
    EXPECT_EQ(edge3->source, 1);
    EXPECT_EQ(edge3->target, 0);
    EXPECT_DOUBLE_EQ(edge3->weight, 4.0);
    
    auto edge4 = generator.next();
    ASSERT_TRUE(edge4.has_value());
    EXPECT_EQ(edge4->source, 1);
    EXPECT_EQ(edge4->target, 2);
    EXPECT_DOUBLE_EQ(edge4->weight, 3.0);
    
    auto edge5 = generator.next();
    ASSERT_TRUE(edge5.has_value());
    EXPECT_EQ(edge5->source, 2);
    EXPECT_EQ(edge5->target, 0);
    EXPECT_DOUBLE_EQ(edge5->weight, 5.0);
    
    auto edge6 = generator.next();
    ASSERT_TRUE(edge6.has_value());
    EXPECT_EQ(edge6->source, 2);
    EXPECT_EQ(edge6->target, 1);
    EXPECT_DOUBLE_EQ(edge6->weight, 6.0);
    
    auto edge7 = generator.next();
    EXPECT_FALSE(edge7.has_value()); // No more edges
}

TEST_F(EdgeGeneratorTest, SparseGraphUndirectedWithEdges) {
    gsp::SparseGraph graph(3);
    
    // Create a sparse matrix with some edges
    Eigen::SparseMatrix<double> weights(3, 3);
    weights.insert(0, 1) = 1.0;
    weights.insert(1, 0) = 1.0;  // For undirected, both directions exist in matrix
    weights.insert(0, 2) = 2.0;
    weights.insert(2, 0) = 2.0;
    weights.insert(1, 2) = 3.0;
    weights.insert(2, 1) = 3.0;
    
    graph.setWeights(weights, false); // undirected
    
    auto generator = graph.iterEdges();
    
    // Should get edges: (0,1), (0,2), (1,2) - only upper triangle for undirected
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 1);
    EXPECT_DOUBLE_EQ(edge1->weight, 1.0);
    
    auto edge2 = generator.next();
    ASSERT_TRUE(edge2.has_value());
    EXPECT_EQ(edge2->source, 0);
    EXPECT_EQ(edge2->target, 2);
    EXPECT_DOUBLE_EQ(edge2->weight, 2.0);
    
    auto edge3 = generator.next();
    ASSERT_TRUE(edge3.has_value());
    EXPECT_EQ(edge3->source, 1);
    EXPECT_EQ(edge3->target, 2);
    EXPECT_DOUBLE_EQ(edge3->weight, 3.0);
    
    auto edge4 = generator.next();
    EXPECT_FALSE(edge4.has_value()); // No more edges
}

TEST_F(EdgeGeneratorTest, ThresholdFiltering) {
    gsp::DenseGraph graph(3);
    
    // Set up a matrix with different weights
    Eigen::MatrixXd weights(3, 3);
    weights << 0, 0.5, 2.0,
               0.5, 0, 3.0,
               2.0, 3.0, 0;
    
    graph.setWeights(weights, false); // undirected
    
    // Test with threshold of 1.0 - should only get edges with weight > 1.0
    auto generator = graph.iterEdges(1.0);
    
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 2);
    EXPECT_DOUBLE_EQ(edge1->weight, 2.0);
    
    auto edge2 = generator.next();
    ASSERT_TRUE(edge2.has_value());
    EXPECT_EQ(edge2->source, 1);
    EXPECT_EQ(edge2->target, 2);
    EXPECT_DOUBLE_EQ(edge2->weight, 3.0);
    
    auto edge3 = generator.next();
    EXPECT_FALSE(edge3.has_value()); // No more edges above threshold
}

TEST_F(EdgeGeneratorTest, ResetFunctionality) {
    gsp::DenseGraph graph(3);
    
    // Set up a simple adjacency matrix
    Eigen::MatrixXd weights(3, 3);
    weights << 0, 1, 2,
               1, 0, 3,
               2, 3, 0;
    
    graph.setWeights(weights, false); // undirected
    
    auto generator = graph.iterEdges();
    
    // Get first edge
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 1);
    
    // Reset the generator
    generator.reset();
    
    // Get first edge again after reset
    auto edge1_after_reset = generator.next();
    ASSERT_TRUE(edge1_after_reset.has_value());
    EXPECT_EQ(edge1_after_reset->source, 0);
    EXPECT_EQ(edge1_after_reset->target, 1);
    EXPECT_DOUBLE_EQ(edge1_after_reset->weight, 1.0);
}

TEST_F(EdgeGeneratorTest, SparseGraphDirectedWithEdges) {
    gsp::SparseGraph graph(3);
    
    // Create a sparse matrix with directed edges
    Eigen::SparseMatrix<double> weights(3, 3);
    weights.insert(0, 1) = 1.0;
    weights.insert(0, 2) = 2.0;
    weights.insert(1, 2) = 3.0;
    
    graph.setWeights(weights, true); // directed
    
    auto generator = graph.iterEdges();
    
    // Should get all edges: (0,1), (0,2), (1,2)
    auto edge1 = generator.next();
    ASSERT_TRUE(edge1.has_value());
    EXPECT_EQ(edge1->source, 0);
    EXPECT_EQ(edge1->target, 1);
    EXPECT_DOUBLE_EQ(edge1->weight, 1.0);
    
    auto edge2 = generator.next();
    ASSERT_TRUE(edge2.has_value());
    EXPECT_EQ(edge2->source, 0);
    EXPECT_EQ(edge2->target, 2);
    EXPECT_DOUBLE_EQ(edge2->weight, 2.0);
    
    auto edge3 = generator.next();
    ASSERT_TRUE(edge3.has_value());
    EXPECT_EQ(edge3->source, 1);
    EXPECT_EQ(edge3->target, 2);
    EXPECT_DOUBLE_EQ(edge3->weight, 3.0);
    
    auto edge4 = generator.next();
    EXPECT_FALSE(edge4.has_value()); // No more edges
}

TEST_F(EdgeGeneratorTest, ResetWithThreshold) {
    gsp::DenseGraph graph(3);

    // Set up a matrix with different weights
    Eigen::MatrixXd weights(3, 3);
    weights << 0, 0.5, 2.0,
               0.5, 0, 3.0,
               2.0, 3.0, 0;

    graph.setWeights(weights, false); // undirected

    // Test with threshold of 2.5 directly instead of resetting
    auto generator = graph.iterEdges(2.5);

    auto edge_after_reset = generator.next();
    ASSERT_TRUE(edge_after_reset.has_value());
    EXPECT_EQ(edge_after_reset->source, 1);
    EXPECT_EQ(edge_after_reset->target, 2);
    EXPECT_DOUBLE_EQ(edge_after_reset->weight, 3.0);

    auto edge2 = generator.next();
    EXPECT_FALSE(edge2.has_value()); // No more edges above threshold 2.5
}