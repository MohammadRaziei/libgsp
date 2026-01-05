#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "libgsp/Graph.h"

class GraphDuplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test graph with some data
        original_graph = std::make_unique<gsp::DenseGraph>(3);

        // Set coordinates and names for the original graph
        original_graph->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        original_graph->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        original_graph->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));

        std::vector<std::string> names = {"node0", "node1", "node2"};
        original_graph->setNames(names);

        // Set some edges to make sure they get copied too
        std::vector<gsp::Edge> edges = {{0, 1, 1.5}, {1, 2, 2.5}};
        original_graph->setEdges(edges, false); // undirected
    }

    std::unique_ptr<gsp::DenseGraph> original_graph;
};

// Test the clone() method (value-based, preserves concrete type)
TEST_F(GraphDuplicationTest, CloneMethodCreatesDeepCopy) {
    // Clone the graph using the value-based clone method
    auto cloned_graph = original_graph->clone();

    // Verify that the clone has the same properties
    ASSERT_EQ(cloned_graph->num_nodes, original_graph->num_nodes);
    
    // Check that coordinates are the same
    auto orig_coord_0 = original_graph->coord(0);
    auto clone_coord_0 = cloned_graph->coord(0);
    EXPECT_DOUBLE_EQ(orig_coord_0.x(), clone_coord_0.x());
    EXPECT_DOUBLE_EQ(orig_coord_0.y(), clone_coord_0.y());
    EXPECT_DOUBLE_EQ(orig_coord_0.z(), clone_coord_0.z());

    // Check that names are the same
    auto orig_name_0 = original_graph->name(0);
    auto clone_name_0 = cloned_graph->name(0);
    EXPECT_EQ(orig_name_0, clone_name_0);

    // Verify that they are actually different objects (deep copy)
    bool same_address = (original_graph.get() == cloned_graph.get());
    EXPECT_FALSE(same_address);

    // Test that modifying the original doesn't affect the clone
    original_graph->setCoord(0, gsp::Coord(10.0, 10.0, 10.0));
    auto new_orig_coord_0 = original_graph->coord(0);
    auto still_clone_coord_0 = cloned_graph->coord(0);
    
    EXPECT_DOUBLE_EQ(new_orig_coord_0.x(), 10.0);
    EXPECT_DOUBLE_EQ(still_clone_coord_0.x(), 0.0); // Should remain unchanged
    EXPECT_DOUBLE_EQ(still_clone_coord_0.y(), 0.0);
    EXPECT_DOUBLE_EQ(still_clone_coord_0.z(), 0.0);
}

// Test the copy() method (polymorphic, returns BaseGraph)
TEST_F(GraphDuplicationTest, CopyMethodCreatesDeepCopy) {
    // Copy the graph using the polymorphic copy method
    std::unique_ptr<gsp::BaseGraph> copied_graph = original_graph->copy();

    // Verify that the copy has the same properties
    ASSERT_EQ(copied_graph->num_nodes, original_graph->num_nodes);
    
    // Check that coordinates are the same (using BaseGraph interface)
    auto orig_coord_0 = original_graph->coord(0);
    auto copy_coord_0 = copied_graph->coord(0);
    EXPECT_DOUBLE_EQ(orig_coord_0.x(), copy_coord_0.x());
    EXPECT_DOUBLE_EQ(orig_coord_0.y(), copy_coord_0.y());
    EXPECT_DOUBLE_EQ(orig_coord_0.z(), copy_coord_0.z());

    // Check that names are the same
    auto orig_name_0 = original_graph->name(0);
    auto copy_name_0 = copied_graph->name(0);
    EXPECT_EQ(orig_name_0, copy_name_0);

    // Verify that they are actually different objects (deep copy)
    bool same_address = (original_graph.get() == copied_graph.get());
    EXPECT_FALSE(same_address);

    // Test that modifying the original doesn't affect the copy
    original_graph->setCoord(0, gsp::Coord(20.0, 20.0, 20.0));
    auto new_orig_coord_0 = original_graph->coord(0);
    auto still_copy_coord_0 = copied_graph->coord(0);
    
    EXPECT_DOUBLE_EQ(new_orig_coord_0.x(), 20.0);
    EXPECT_DOUBLE_EQ(still_copy_coord_0.x(), 0.0); // Should remain unchanged
    EXPECT_DOUBLE_EQ(still_copy_coord_0.y(), 0.0);
    EXPECT_DOUBLE_EQ(still_copy_coord_0.z(), 0.0);
}

// Test that both methods work with sparse graphs too
TEST(GraphDuplicationSparseTest, CloneAndCopyWithSparseGraph) {
    // Create a sparse graph
    gsp::SparseGraph sparse_original(4);
    
    // Set some data
    sparse_original.setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
    sparse_original.setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
    std::vector<std::string> names = {"snode0", "snode1", "snode2", "snode3"};
    sparse_original.setNames(names);

    // Set some edges
    std::vector<gsp::Edge> edges = {{0, 1, 1.0}, {2, 3, 2.0}};
    sparse_original.setEdges(edges, true); // directed

    // Test clone method
    auto sparse_cloned = sparse_original.clone();
    ASSERT_EQ(sparse_cloned->num_nodes, sparse_original.num_nodes);
    EXPECT_EQ(sparse_cloned->name(0), "snode0");

    // Test copy method (polymorphic)
    std::unique_ptr<gsp::BaseGraph> sparse_copied = sparse_original.copy();
    ASSERT_EQ(sparse_copied->num_nodes, sparse_original.num_nodes);
    EXPECT_EQ(sparse_copied->name(0), "snode0");

    // Verify independence
    sparse_original.setCoord(0, gsp::Coord(100.0, 100.0, 100.0));
    EXPECT_DOUBLE_EQ(sparse_original.coord(0).x(), 100.0);
    EXPECT_DOUBLE_EQ(sparse_cloned->coord(0).x(), 0.0);  // Should be unchanged
    EXPECT_DOUBLE_EQ(sparse_copied->coord(0).x(), 0.0);  // Should be unchanged
}

// Test that the concrete type is preserved with clone() but not with copy()
TEST_F(GraphDuplicationTest, TypePreservationDifference) {
    // Using clone() preserves the concrete type, so we can access Graph-specific methods
    auto cloned_dense = original_graph->clone();
    // We can call Graph-specific methods on the cloned object
    auto weights = cloned_dense->weights();
    EXPECT_EQ(weights.rows(), 3);
    EXPECT_EQ(weights.cols(), 3);

    // Using copy() returns BaseGraph*, so we lose direct access to Graph-specific methods
    std::unique_ptr<gsp::BaseGraph> copied_base = original_graph->copy();
    // We can only access BaseGraph interface methods
    EXPECT_EQ(copied_base->num_nodes, 3);
    
    // Verify that both objects are independent
    original_graph->setCoord(1, gsp::Coord(99.0, 99.0, 99.0));
    EXPECT_DOUBLE_EQ(cloned_dense->coord(1).x(), 1.0);  // Unchanged
    EXPECT_DOUBLE_EQ(copied_base->coord(1).x(), 1.0);   // Unchanged
}

// Test that internal caches are properly invalidated in clones
TEST_F(GraphDuplicationTest, CacheInvalidationInClone) {
    // Access some computed properties to potentially populate caches
    auto& laplacian = original_graph->laplacian();
    EXPECT_EQ(laplacian.rows(), 3);
    EXPECT_EQ(laplacian.cols(), 3);

    // Clone the graph - this should not copy any cached values
    auto cloned_graph = original_graph->clone();

    // The cloned graph should compute its own laplacian independently
    auto& cloned_laplacian = cloned_graph->laplacian();
    EXPECT_EQ(cloned_laplacian.rows(), 3);
    EXPECT_EQ(cloned_laplacian.cols(), 3);

    // Verify independence by modifying original and checking clone is unaffected
    std::vector<gsp::Edge> new_edges = {{0, 2, 3.0}};
    original_graph->setEdges(new_edges, false);
    
    auto& modified_laplacian = original_graph->laplacian();
    
    // The cloned laplacian should still be based on the original edges, not the new ones
    // This verifies that the cache was properly invalidated/separated
    EXPECT_NE(&cloned_laplacian, &modified_laplacian);  // Different objects
    EXPECT_EQ(cloned_graph->edges().size(), 2);  // Should have original 2 edges
    EXPECT_EQ(original_graph->edges().size(), 1);  // Should have new 1 edge
}