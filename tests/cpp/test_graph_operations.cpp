#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "libgsp/Graph.h"

class GraphOperationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test graph with some data
        original_dense = std::make_unique<gsp::DenseGraph>(3);
        original_sparse = std::make_unique<gsp::SparseGraph>(3);

        // Set coordinates and names for the original graphs
        original_dense->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        original_dense->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        original_dense->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));

        original_sparse->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        original_sparse->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        original_sparse->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));

        std::vector<std::string> names = {"node0", "node1", "node2"};
        original_dense->setNames(names);
        original_sparse->setNames(names);

        // Set some edges to make sure they get processed
        std::vector<gsp::Edge> edges = {{0, 1, 1.5}, {1, 2, 2.5}};
        original_dense->setEdges(edges, false); // undirected
        original_sparse->setEdges(edges, false); // undirected
    }

    std::unique_ptr<gsp::DenseGraph> original_dense;
    std::unique_ptr<gsp::SparseGraph> original_sparse;
};

// Test move constructor
TEST_F(GraphOperationsTest, MoveConstructor) {
    gsp::DenseGraph original(3);
    original.setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
    original.setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
    original.setCoord(2, gsp::Coord(2.0, 2.0, 2.0));

    std::vector<std::string> names = {"node0", "node1", "node2"};
    original.setNames(names);

    std::vector<gsp::Edge> edges = {{0, 1, 1.5}, {1, 2, 2.5}};
    original.setEdges(edges, false);

    // Test move constructor
    gsp::DenseGraph moved_graph = std::move(original);

    // Verify that moved_graph has the correct data
    ASSERT_EQ(moved_graph.num_nodes, 3);
    EXPECT_EQ(moved_graph.name(0), "node0");
    EXPECT_EQ(moved_graph.name(1), "node1");
    EXPECT_EQ(moved_graph.name(2), "node2");

    auto coord0 = moved_graph.coord(0);
    EXPECT_DOUBLE_EQ(coord0.x(), 0.0);
    EXPECT_DOUBLE_EQ(coord0.y(), 0.0);
    EXPECT_DOUBLE_EQ(coord0.z(), 0.0);

    auto edges_moved = moved_graph.edges();
    EXPECT_EQ(edges_moved.size(), 2);
}

// Test move assignment operator
TEST_F(GraphOperationsTest, MoveAssignmentOperator) {
    // Create source graph
    gsp::DenseGraph source(2);
    source.setCoord(0, gsp::Coord(10.0, 10.0, 10.0));
    source.setCoord(1, gsp::Coord(20.0, 20.0, 20.0));

    std::vector<std::string> source_names = {"src0", "src1"};
    source.setNames(source_names);

    std::vector<gsp::Edge> source_edges = {{0, 1, 3.0}};
    source.setEdges(source_edges, true);

    // Create destination graph with different data
    gsp::DenseGraph dest(4);
    dest.setCoord(0, gsp::Coord(100.0, 100.0, 100.0));
    dest.setCoord(1, gsp::Coord(200.0, 200.0, 200.0));

    std::vector<std::string> dest_names = {"dest0", "dest1"};
    dest.setNames(dest_names);

    // Verify initial state of destination
    EXPECT_EQ(dest.num_nodes, 4);
    EXPECT_EQ(dest.name(0), "dest0");

    // Perform move assignment
    dest = std::move(source);

    // Verify that dest now has source's data
    EXPECT_EQ(dest.num_nodes, 2);
    EXPECT_EQ(dest.name(0), "src0");
    EXPECT_EQ(dest.name(1), "src1");

    auto new_coord0 = dest.coord(0);
    EXPECT_DOUBLE_EQ(new_coord0.x(), 10.0);
    EXPECT_DOUBLE_EQ(new_coord0.y(), 10.0);
    EXPECT_DOUBLE_EQ(new_coord0.z(), 10.0);

    auto edges_moved = dest.edges();
    EXPECT_EQ(edges_moved.size(), 1);
}

// Test self move assignment (should be safe)
TEST_F(GraphOperationsTest, SelfMoveAssignment) {
    gsp::DenseGraph graph(2);
    graph.setCoord(0, gsp::Coord(5.0, 5.0, 5.0));
    graph.setCoord(1, gsp::Coord(6.0, 6.0, 6.0));

    std::vector<std::string> names = {"test0", "test1"};
    graph.setNames(names);

    std::vector<gsp::Edge> edges = {{0, 1, 1.0}};
    graph.setEdges(edges, false);

    // Store original values
    auto original_coord0 = graph.coord(0);
    auto original_coord1 = graph.coord(1);
    std::string original_name0 = graph.name(0);
    std::string original_name1 = graph.name(1);

    // Self move assignment should be safe
    graph = std::move(graph);

    // Values should remain the same
    EXPECT_EQ(graph.num_nodes, 2);
    EXPECT_EQ(graph.name(0), original_name0);
    EXPECT_EQ(graph.name(1), original_name1);

    auto coord0_after = graph.coord(0);
    EXPECT_DOUBLE_EQ(coord0_after.x(), original_coord0.x());
    EXPECT_DOUBLE_EQ(coord0_after.y(), original_coord0.y());
    EXPECT_DOUBLE_EQ(coord0_after.z(), original_coord0.z());
}

// Test clone method (returns unique_ptr)
TEST_F(GraphOperationsTest, CloneMethod) {
    // Test clone on dense graph
    auto cloned_dense = original_dense->clone();
    
    // Verify that cloned graph has same properties
    ASSERT_EQ(cloned_dense->num_nodes, original_dense->num_nodes);
    EXPECT_EQ(cloned_dense->name(0), original_dense->name(0));
    EXPECT_EQ(cloned_dense->name(1), original_dense->name(1));
    EXPECT_EQ(cloned_dense->name(2), original_dense->name(2));

    auto orig_coord0 = original_dense->coord(0);
    auto clone_coord0 = cloned_dense->coord(0);
    EXPECT_DOUBLE_EQ(orig_coord0.x(), clone_coord0.x());
    EXPECT_DOUBLE_EQ(orig_coord0.y(), clone_coord0.y());
    EXPECT_DOUBLE_EQ(orig_coord0.z(), clone_coord0.z());

    // Verify independence - modifying original shouldn't affect clone
    original_dense->setCoord(0, gsp::Coord(99.0, 99.0, 99.0));
    EXPECT_DOUBLE_EQ(original_dense->coord(0).x(), 99.0);
    EXPECT_DOUBLE_EQ(cloned_dense->coord(0).x(), 0.0); // Should remain unchanged

    // Test clone on sparse graph
    auto cloned_sparse = original_sparse->clone();
    ASSERT_EQ(cloned_sparse->num_nodes, original_sparse->num_nodes);
    EXPECT_EQ(cloned_sparse->name(0), original_sparse->name(0));
    
    // Verify independence
    original_sparse->setCoord(1, gsp::Coord(88.0, 88.0, 88.0));
    EXPECT_DOUBLE_EQ(original_sparse->coord(1).x(), 88.0);
    EXPECT_DOUBLE_EQ(cloned_sparse->coord(1).x(), 1.0); // Should remain unchanged
}

// Test toSparse method (returns unique_ptr)
TEST_F(GraphOperationsTest, ToSparseMethod) {
    // Test Dense -> Sparse conversion
    auto sparse_converted = original_dense->toSparse();
    
    ASSERT_EQ(sparse_converted->num_nodes, original_dense->num_nodes);
    EXPECT_EQ(sparse_converted->name(0), original_dense->name(0));
    EXPECT_EQ(sparse_converted->name(1), original_dense->name(1));
    EXPECT_EQ(sparse_converted->name(2), original_dense->name(2));

    // Check coordinates
    auto orig_coord1 = original_dense->coord(1);
    auto sparse_coord1 = sparse_converted->coord(1);
    EXPECT_DOUBLE_EQ(orig_coord1.x(), sparse_coord1.x());
    EXPECT_DOUBLE_EQ(orig_coord1.y(), sparse_coord1.y());
    EXPECT_DOUBLE_EQ(orig_coord1.z(), sparse_coord1.z());

    // Check edges
    auto orig_edges = original_dense->edges();
    auto sparse_edges = sparse_converted->edges();
    EXPECT_EQ(orig_edges.size(), sparse_edges.size());

    // Verify independence
    original_dense->setCoord(2, gsp::Coord(77.0, 77.0, 77.0));
    EXPECT_DOUBLE_EQ(original_dense->coord(2).x(), 77.0);
    EXPECT_DOUBLE_EQ(sparse_converted->coord(2).x(), 2.0); // Should remain unchanged

    // Test no-op conversion (Sparse -> Sparse should return clone with warning)
    auto sparse_to_sparse = original_sparse->toSparse();
    ASSERT_EQ(sparse_to_sparse->num_nodes, original_sparse->num_nodes);
    EXPECT_EQ(sparse_to_sparse->name(0), original_sparse->name(0));
    
    // Verify independence
    original_sparse->setCoord(0, gsp::Coord(66.0, 66.0, 66.0));
    EXPECT_DOUBLE_EQ(original_sparse->coord(0).x(), 66.0);
    EXPECT_DOUBLE_EQ(sparse_to_sparse->coord(0).x(), 0.0); // Should remain unchanged
}

// Test toDense method (returns unique_ptr)
TEST_F(GraphOperationsTest, ToDenseMethod) {
    // Test Sparse -> Dense conversion
    auto dense_converted = original_sparse->toDense();
    
    ASSERT_EQ(dense_converted->num_nodes, original_sparse->num_nodes);
    EXPECT_EQ(dense_converted->name(0), original_sparse->name(0));
    EXPECT_EQ(dense_converted->name(1), original_sparse->name(1));
    EXPECT_EQ(dense_converted->name(2), original_sparse->name(2));

    // Check coordinates
    auto orig_coord2 = original_sparse->coord(2);
    auto dense_coord2 = dense_converted->coord(2);
    EXPECT_DOUBLE_EQ(orig_coord2.x(), dense_coord2.x());
    EXPECT_DOUBLE_EQ(orig_coord2.y(), dense_coord2.y());
    EXPECT_DOUBLE_EQ(orig_coord2.z(), dense_coord2.z());

    // Check edges
    auto orig_edges = original_sparse->edges();
    auto dense_edges = dense_converted->edges();
    EXPECT_EQ(orig_edges.size(), dense_edges.size());

    // Verify independence
    original_sparse->setCoord(1, gsp::Coord(55.0, 55.0, 55.0));
    EXPECT_DOUBLE_EQ(original_sparse->coord(1).x(), 55.0);
    EXPECT_DOUBLE_EQ(dense_converted->coord(1).x(), 1.0); // Should remain unchanged

    // Test no-op conversion (Dense -> Dense should return clone with warning)
    auto dense_to_dense = original_dense->toDense();
    ASSERT_EQ(dense_to_dense->num_nodes, original_dense->num_nodes);
    EXPECT_EQ(dense_to_dense->name(0), original_dense->name(0));
    
    // Verify independence
    original_dense->setCoord(0, gsp::Coord(44.0, 44.0, 44.0));
    EXPECT_DOUBLE_EQ(original_dense->coord(0).x(), 44.0);
    EXPECT_DOUBLE_EQ(dense_to_dense->coord(0).x(), 0.0); // Should remain unchanged
}

// Test copy method (polymorphic, returns BaseGraph unique_ptr)
TEST_F(GraphOperationsTest, CopyMethod) {
    // Test copy on dense graph
    std::unique_ptr<gsp::BaseGraph> copied_dense = original_dense->copy();
    
    ASSERT_EQ(copied_dense->num_nodes, original_dense->num_nodes);
    EXPECT_EQ(copied_dense->name(0), original_dense->name(0));
    EXPECT_EQ(copied_dense->name(1), original_dense->name(1));

    auto orig_coord0 = original_dense->coord(0);
    auto copy_coord0 = copied_dense->coord(0);
    EXPECT_DOUBLE_EQ(orig_coord0.x(), copy_coord0.x());
    EXPECT_DOUBLE_EQ(orig_coord0.y(), copy_coord0.y());
    EXPECT_DOUBLE_EQ(orig_coord0.z(), copy_coord0.z());

    // Verify independence
    original_dense->setCoord(0, gsp::Coord(33.0, 33.0, 33.0));
    EXPECT_DOUBLE_EQ(original_dense->coord(0).x(), 33.0);
    EXPECT_DOUBLE_EQ(copied_dense->coord(0).x(), 0.0); // Should remain unchanged

    // Test copy on sparse graph
    std::unique_ptr<gsp::BaseGraph> copied_sparse = original_sparse->copy();
    ASSERT_EQ(copied_sparse->num_nodes, original_sparse->num_nodes);
    EXPECT_EQ(copied_sparse->name(0), original_sparse->name(0));
    
    // Verify independence
    original_sparse->setCoord(2, gsp::Coord(22.0, 22.0, 22.0));
    EXPECT_DOUBLE_EQ(original_sparse->coord(2).x(), 22.0);
    EXPECT_DOUBLE_EQ(copied_sparse->coord(2).x(), 2.0); // Should remain unchanged
}

// Test that conversion methods create independent objects
TEST_F(GraphOperationsTest, ConversionIndependence) {
    // Create original graph
    gsp::DenseGraph original(2);
    original.setCoord(0, gsp::Coord(1.0, 1.0, 1.0));
    original.setCoord(1, gsp::Coord(2.0, 2.0, 2.0));
    
    std::vector<std::string> names = {"orig0", "orig1"};
    original.setNames(names);
    
    std::vector<gsp::Edge> edges = {{0, 1, 1.0}};
    original.setEdges(edges, false);

    // Convert to sparse
    auto sparse_version = original.toSparse();
    
    // Convert back to dense
    auto dense_converted_back = sparse_version->toDense();

    // Modify original
    original.setCoord(0, gsp::Coord(99.0, 99.0, 99.0));
    std::vector<gsp::Edge> new_edges = {{0, 1, 5.0}};
    original.setEdges(new_edges, false);

    // Verify all versions are independent
    EXPECT_DOUBLE_EQ(original.coord(0).x(), 99.0);
    EXPECT_DOUBLE_EQ(sparse_version->coord(0).x(), 1.0);  // Should be unchanged
    EXPECT_DOUBLE_EQ(dense_converted_back->coord(0).x(), 1.0);  // Should be unchanged

    EXPECT_EQ(original.edges().size(), 1);  // New edge added
    EXPECT_EQ(sparse_version->edges().size(), 1);  // Original edge
    EXPECT_EQ(dense_converted_back->edges().size(), 1);  // Original edge
}

// Test mixed operations with smart pointers
TEST_F(GraphOperationsTest, MixedOperationsWithSmartPointers) {
    // Start with dense graph
    auto dense_ptr = std::make_unique<gsp::DenseGraph>(2);
    dense_ptr->setCoord(0, gsp::Coord(1.0, 1.0, 1.0));
    dense_ptr->setCoord(1, gsp::Coord(2.0, 2.0, 2.0));

    std::vector<std::string> names = {"ptr0", "ptr1"};
    dense_ptr->setNames(names);

    // Clone it
    auto cloned_ptr = dense_ptr->clone();

    // Convert to sparse
    auto sparse_ptr = dense_ptr->toSparse();

    // Convert cloned to dense (no-op)
    auto dense_again_ptr = cloned_ptr->toDense();

    // Verify all are independent
    dense_ptr->setCoord(0, gsp::Coord(10.0, 10.0, 10.0));

    EXPECT_DOUBLE_EQ(dense_ptr->coord(0).x(), 10.0);
    EXPECT_DOUBLE_EQ(cloned_ptr->coord(0).x(), 1.0);  // Unchanged
    EXPECT_DOUBLE_EQ(sparse_ptr->coord(0).x(), 1.0);  // Unchanged
    EXPECT_DOUBLE_EQ(dense_again_ptr->coord(0).x(), 1.0);  // Unchanged

    // Verify names are preserved
    EXPECT_EQ(dense_ptr->name(0), "ptr0");
    EXPECT_EQ(cloned_ptr->name(0), "ptr0");
    EXPECT_EQ(sparse_ptr->name(0), "ptr0");
    EXPECT_EQ(dense_again_ptr->name(0), "ptr0");
}

// Test threshold functionality in toSparse and toDense methods
TEST_F(GraphOperationsTest, ThresholdFunctionality) {
    // Create a dense graph with various edge weights
    gsp::DenseGraph dense_graph(4);

    // Add edges with different weights
    std::vector<gsp::Edge> edges = {
        gsp::Edge(0, 1, 0.8),  // High weight
        gsp::Edge(1, 2, 0.3),  // Low weight
        gsp::Edge(2, 3, 0.6),  // Medium weight
        gsp::Edge(0, 3, 0.1)   // Very low weight
    };

    dense_graph.setEdges(edges, true);  // directed

    // Test toSparse with threshold 0.5 (should exclude edges with weight <= 0.5)
    auto sparse_thresh = dense_graph.toSparse(0.5);
    auto sparse_edges_thresh = sparse_thresh->edges();

    // Should only have edges with weight > 0.5: (0,1,0.8) and (2,3,0.6)
    EXPECT_EQ(sparse_edges_thresh.size(), 2);
    for (const auto& edge : sparse_edges_thresh) {
        EXPECT_GT(edge.weight, 0.5);
    }

    // Test toSparse with threshold 0.0 (should include all edges)
    auto sparse_no_thresh = dense_graph.toSparse(0.0);
    auto sparse_edges_no_thresh = sparse_no_thresh->edges();

    EXPECT_EQ(sparse_edges_no_thresh.size(), 4);  // All original edges

    // Test toDense with threshold 0.4 (should exclude edges with weight <= 0.4)
    auto sparse_for_test = dense_graph.toSparse(0.0);  // First convert to sparse
    auto dense_thresh = sparse_for_test->toDense(0.4);
    auto dense_edges_thresh = dense_thresh->edges();

    // Should only have edges with weight > 0.4
    EXPECT_EQ(dense_edges_thresh.size(), 2);  // (0,1,0.8) and (2,3,0.6)
    for (const auto& edge : dense_edges_thresh) {
        EXPECT_GT(edge.weight, 0.4);
    }

    // Test toDense with threshold 0.0 (should include all edges)
    auto dense_no_thresh = sparse_for_test->toDense(0.0);
    auto dense_edges_no_thresh = dense_no_thresh->edges();

    EXPECT_EQ(dense_edges_no_thresh.size(), 4);  // All edges
}