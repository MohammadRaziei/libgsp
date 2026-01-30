#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "libgsp/Graph.h"

class GraphMoveTest : public ::testing::Test {
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

        // Set some edges to make sure they get moved too
        std::vector<gsp::Edge> edges = {{0, 1, 1.5}, {1, 2, 2.5}};
        original_graph->setEdges(edges, false); // undirected
    }

    std::unique_ptr<gsp::DenseGraph> original_graph;
};

// Test move constructor
TEST_F(GraphMoveTest, MoveConstructor) {
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
    ASSERT_EQ(moved_graph.numNodes(), 3);
    EXPECT_EQ(moved_graph.name(0), "node0");
    EXPECT_EQ(moved_graph.name(1), "node1");
    EXPECT_EQ(moved_graph.name(2), "node2");

    auto coord0 = moved_graph.coord(0);
    EXPECT_DOUBLE_EQ(coord0.x(), 0.0);
    EXPECT_DOUBLE_EQ(coord0.y(), 0.0);
    EXPECT_DOUBLE_EQ(coord0.z(), 0.0);

    auto coord1 = moved_graph.coord(1);
    EXPECT_DOUBLE_EQ(coord1.x(), 1.0);
    EXPECT_DOUBLE_EQ(coord1.y(), 1.0);
    EXPECT_DOUBLE_EQ(coord1.z(), 1.0);

    auto coord2 = moved_graph.coord(2);
    EXPECT_DOUBLE_EQ(coord2.x(), 2.0);
    EXPECT_DOUBLE_EQ(coord2.y(), 2.0);
    EXPECT_DOUBLE_EQ(coord2.z(), 2.0);

    auto edges_moved = moved_graph.edges();
    EXPECT_EQ(edges_moved.size(), 2);
}

// Test move assignment operator
TEST_F(GraphMoveTest, MoveAssignmentOperator) {
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
    dest.setCoord(2, gsp::Coord(300.0, 300.0, 300.0));
    dest.setCoord(3, gsp::Coord(400.0, 400.0, 400.0));

    std::vector<std::string> dest_names = {"dest0", "dest1", "dest2", "dest3"};
    dest.setNames(dest_names);

    // Verify initial state of destination
    EXPECT_EQ(dest.numNodes(), 4);
    EXPECT_EQ(dest.name(0), "dest0");
    auto initial_coord = dest.coord(0);
    EXPECT_DOUBLE_EQ(initial_coord.x(), 100.0);

    // Perform move assignment
    dest = std::move(source);

    // Verify that dest now has source's data
    EXPECT_EQ(dest.numNodes(), 2);
    EXPECT_EQ(dest.name(0), "src0");
    EXPECT_EQ(dest.name(1), "src1");

    auto new_coord0 = dest.coord(0);
    EXPECT_DOUBLE_EQ(new_coord0.x(), 10.0);
    EXPECT_DOUBLE_EQ(new_coord0.y(), 10.0);
    EXPECT_DOUBLE_EQ(new_coord0.z(), 10.0);

    auto new_coord1 = dest.coord(1);
    EXPECT_DOUBLE_EQ(new_coord1.x(), 20.0);
    EXPECT_DOUBLE_EQ(new_coord1.y(), 20.0);
    EXPECT_DOUBLE_EQ(new_coord1.z(), 20.0);

    auto edges_moved = dest.edges();
    EXPECT_EQ(edges_moved.size(), 1);
    EXPECT_EQ(edges_moved[0].source(), 0);
    EXPECT_EQ(edges_moved[0].target(), 1);
    EXPECT_DOUBLE_EQ(edges_moved[0].weight(), 3.0);
}

// Test self move assignment (should be safe)
TEST_F(GraphMoveTest, SelfMoveAssignment) {
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
    auto original_edges = graph.edges();

    // Self move assignment should be safe
    graph = std::move(graph);

    // Values should remain the same
    EXPECT_EQ(graph.numNodes(), 2);
    EXPECT_EQ(graph.name(0), original_name0);
    EXPECT_EQ(graph.name(1), original_name1);

    auto coord0_after = graph.coord(0);
    EXPECT_DOUBLE_EQ(coord0_after.x(), original_coord0.x());
    EXPECT_DOUBLE_EQ(coord0_after.y(), original_coord0.y());
    EXPECT_DOUBLE_EQ(coord0_after.z(), original_coord0.z());

    auto coord1_after = graph.coord(1);
    EXPECT_DOUBLE_EQ(coord1_after.x(), original_coord1.x());
    EXPECT_DOUBLE_EQ(coord1_after.y(), original_coord1.y());
    EXPECT_DOUBLE_EQ(coord1_after.z(), original_coord1.z());

    auto edges_after = graph.edges();
    EXPECT_EQ(edges_after.size(), original_edges.size());
    if (!original_edges.empty() && !edges_after.empty()) {
        EXPECT_EQ(edges_after[0].source(), original_edges[0].source());
        EXPECT_EQ(edges_after[0].target(), original_edges[0].target());
        EXPECT_DOUBLE_EQ(edges_after[0].weight(), original_edges[0].weight());
    }
}

// Test move operations with sparse graphs
TEST_F(GraphMoveTest, SparseGraphMoveOperations) {
    // Test move constructor with sparse graph
    gsp::SparseGraph sparse_orig(3);
    sparse_orig.setCoord(0, gsp::Coord(1.0, 2.0, 3.0));
    sparse_orig.setCoord(1, gsp::Coord(4.0, 5.0, 6.0));
    sparse_orig.setCoord(2, gsp::Coord(7.0, 8.0, 9.0));

    std::vector<std::string> names = {"sparse0", "sparse1", "sparse2"};
    sparse_orig.setNames(names);

    std::vector<gsp::Edge> edges = {{0, 1, 1.0}, {1, 2, 2.0}};
    sparse_orig.setEdges(edges, false);

    // Move construct
    gsp::SparseGraph sparse_moved = std::move(sparse_orig);

    // Verify moved graph has correct data
    EXPECT_EQ(sparse_moved.numNodes(), 3);
    EXPECT_EQ(sparse_moved.name(0), "sparse0");
    EXPECT_EQ(sparse_moved.name(1), "sparse1");
    EXPECT_EQ(sparse_moved.name(2), "sparse2");

    auto coord0 = sparse_moved.coord(0);
    EXPECT_DOUBLE_EQ(coord0.x(), 1.0);
    EXPECT_DOUBLE_EQ(coord0.y(), 2.0);
    EXPECT_DOUBLE_EQ(coord0.z(), 3.0);

    auto edges_moved = sparse_moved.edges();
    EXPECT_EQ(edges_moved.size(), 2);
}

// Test move assignment with sparse graphs
TEST_F(GraphMoveTest, SparseGraphMoveAssignment) {
    // Create source sparse graph
    gsp::SparseGraph source(2);
    source.setCoord(0, gsp::Coord(100.0, 200.0, 300.0));
    source.setCoord(1, gsp::Coord(400.0, 500.0, 600.0));

    std::vector<std::string> source_names = {"src_sparse0", "src_sparse1"};
    source.setNames(source_names);

    std::vector<gsp::Edge> source_edges = {{0, 1, 5.0}};
    source.setEdges(source_edges, true);

    // Create destination sparse graph
    gsp::SparseGraph dest(3);
    dest.setCoord(0, gsp::Coord(1.0, 1.0, 1.0));
    dest.setCoord(1, gsp::Coord(2.0, 2.0, 2.0));
    dest.setCoord(2, gsp::Coord(3.0, 3.0, 3.0));

    std::vector<std::string> dest_names = {"dest_sparse0", "dest_sparse1", "dest_sparse2"};
    dest.setNames(dest_names);

    // Verify initial state
    EXPECT_EQ(dest.numNodes(), 3);
    EXPECT_EQ(dest.name(0), "dest_sparse0");

    // Perform move assignment
    dest = std::move(source);

    // Verify that dest now has source's data
    EXPECT_EQ(dest.numNodes(), 2);
    EXPECT_EQ(dest.name(0), "src_sparse0");
    EXPECT_EQ(dest.name(1), "src_sparse1");

    auto coord0 = dest.coord(0);
    EXPECT_DOUBLE_EQ(coord0.x(), 100.0);
    EXPECT_DOUBLE_EQ(coord0.y(), 200.0);
    EXPECT_DOUBLE_EQ(coord0.z(), 300.0);

    auto edges_moved = dest.edges();
    EXPECT_EQ(edges_moved.size(), 1);
    EXPECT_EQ(edges_moved[0].source(), 0);
    EXPECT_EQ(edges_moved[0].target(), 1);
    EXPECT_DOUBLE_EQ(edges_moved[0].weight(), 5.0);
}