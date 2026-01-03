//
// Created by Mohammad on 1/3/2026.
//

#include <gtest/gtest.h>
#include <vector>

#include "libgsp/VertexGraph.h"

class VertexIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<gsp::VertexGraph>(3);
        
        // Set coordinates and names for the nodes
        graph->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        graph->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        graph->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));
        
        std::vector<std::string> names = {"node0", "node1", "node2"};
        graph->setNames(names);
    }

    std::unique_ptr<gsp::VertexGraph> graph;
};

TEST_F(VertexIteratorTest, NonConstBeginEnd) {
    // Test non-const begin/end
    auto begin_it = graph->begin();
    auto end_it = graph->end();
    
    ASSERT_NE(begin_it, end_it);  // Should not be equal initially
    
    // Test dereference operator
    EXPECT_EQ(begin_it->id, 0);
    EXPECT_EQ(begin_it->name, "node0");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 0.0);
    
    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 1);
    EXPECT_EQ(begin_it->name, "node1");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 1.0);
    
    // Test post-increment
    auto temp_it = begin_it++;
    EXPECT_EQ(temp_it->id, 1);
    EXPECT_EQ(begin_it->id, 2);
    
    // Test increment to end
    ++begin_it;
    EXPECT_EQ(begin_it, end_it);  // Should now be at end
}

TEST_F(VertexIteratorTest, ConstBeginEnd) {
    const gsp::VertexGraph& const_graph = *graph;
    
    // Test const begin/end
    auto begin_it = const_graph.begin();
    auto end_it = const_graph.end();
    
    ASSERT_NE(begin_it, end_it);  // Should not be equal initially
    
    // Test dereference operator
    EXPECT_EQ(begin_it->id, 0);
    EXPECT_EQ(begin_it->name, "node0");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 0.0);
    
    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 1);
    EXPECT_EQ(begin_it->name, "node1");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 1.0);
    
    // Test increment to end
    ++begin_it;
    ++begin_it;
    EXPECT_EQ(begin_it, end_it);  // Should now be at end
}

TEST_F(VertexIteratorTest, CBeginCEnd) {
    const gsp::VertexGraph& const_graph = *graph;
    
    // Test cbegin/cend
    auto begin_it = const_graph.cbegin();
    auto end_it = const_graph.cend();
    
    ASSERT_NE(begin_it, end_it);  // Should not be equal initially
    
    // Test dereference operator
    EXPECT_EQ(begin_it->id, 0);
    EXPECT_EQ(begin_it->name, "node0");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 0.0);
    
    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 1);
    EXPECT_EQ(begin_it->name, "node1");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 1.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 1.0);
    
    // Test increment to end
    ++begin_it;
    ++begin_it;
    EXPECT_EQ(begin_it, end_it);  // Should now be at end
}

TEST_F(VertexIteratorTest, IteratorEquality) {
    auto it1 = graph->begin();
    auto it2 = graph->begin();
    
    EXPECT_EQ(it1, it2);  // Two iterators at same position should be equal
    
    ++it1;
    EXPECT_NE(it1, it2);  // Different positions should not be equal
    
    ++it2;
    EXPECT_EQ(it1, it2);  // Same position again should be equal
}

TEST_F(VertexIteratorTest, IteratorDereference) {
    auto it = graph->begin();
    
    // Test operator*
    auto& node_ref = *it;
    EXPECT_EQ(node_ref.id, 0);
    EXPECT_EQ(node_ref.name, "node0");
    EXPECT_DOUBLE_EQ(node_ref.coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(node_ref.coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(node_ref.coord.z(), 0.0);
    
    // Test operator->
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
    EXPECT_DOUBLE_EQ(it->coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(it->coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(it->coord.z(), 0.0);
}

TEST_F(VertexIteratorTest, RangeBasedForLoop) {
    std::vector<gsp::Node> nodes;
    
    // Test range-based for with non-const
    for (auto& node : *graph) {
        nodes.push_back(node);
    }
    
    EXPECT_EQ(nodes.size(), 3);
    EXPECT_EQ(nodes[0].id, 0);
    EXPECT_EQ(nodes[0].name, "node0");
    EXPECT_DOUBLE_EQ(nodes[0].coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(nodes[0].coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(nodes[0].coord.z(), 0.0);
    
    EXPECT_EQ(nodes[1].id, 1);
    EXPECT_EQ(nodes[1].name, "node1");
    EXPECT_DOUBLE_EQ(nodes[1].coord.x(), 1.0);
    EXPECT_DOUBLE_EQ(nodes[1].coord.y(), 1.0);
    EXPECT_DOUBLE_EQ(nodes[1].coord.z(), 1.0);
    
    EXPECT_EQ(nodes[2].id, 2);
    EXPECT_EQ(nodes[2].name, "node2");
    EXPECT_DOUBLE_EQ(nodes[2].coord.x(), 2.0);
    EXPECT_DOUBLE_EQ(nodes[2].coord.y(), 2.0);
    EXPECT_DOUBLE_EQ(nodes[2].coord.z(), 2.0);
}

TEST_F(VertexIteratorTest, ConstRangeBasedForLoop) {
    std::vector<gsp::Node> nodes;
    const gsp::VertexGraph& const_graph = *graph;
    
    // Test range-based for with const
    for (const auto& node : const_graph) {
        nodes.push_back(node);
    }
    
    EXPECT_EQ(nodes.size(), 3);
    EXPECT_EQ(nodes[0].id, 0);
    EXPECT_EQ(nodes[0].name, "node0");
    EXPECT_DOUBLE_EQ(nodes[0].coord.x(), 0.0);
    EXPECT_DOUBLE_EQ(nodes[0].coord.y(), 0.0);
    EXPECT_DOUBLE_EQ(nodes[0].coord.z(), 0.0);
    
    EXPECT_EQ(nodes[1].id, 1);
    EXPECT_EQ(nodes[1].name, "node1");
    EXPECT_DOUBLE_EQ(nodes[1].coord.x(), 1.0);
    EXPECT_DOUBLE_EQ(nodes[1].coord.y(), 1.0);
    EXPECT_DOUBLE_EQ(nodes[1].coord.z(), 1.0);
    
    EXPECT_EQ(nodes[2].id, 2);
    EXPECT_EQ(nodes[2].name, "node2");
    EXPECT_DOUBLE_EQ(nodes[2].coord.x(), 2.0);
    EXPECT_DOUBLE_EQ(nodes[2].coord.y(), 2.0);
    EXPECT_DOUBLE_EQ(nodes[2].coord.z(), 2.0);
}

TEST_F(VertexIteratorTest, IteratorDecrement) {
    auto it = graph->begin();
    
    // Move to the second element
    ++it;
    EXPECT_EQ(it->id, 1);
    
    // Move back to the first element
    --it;
    EXPECT_EQ(it->id, 0);
    
    // Test post-decrement
    auto temp_it = it--;
    EXPECT_EQ(temp_it->id, 0);
    // Since we're at position 0, decrementing should not change the iterator
    // (behavior depends on implementation, but we should not crash)
}

TEST_F(VertexIteratorTest, IteratorComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin();
    ++it2;  // it2 is now at position 1
    
    EXPECT_NE(it1, it2);
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
    
    ++it1;  // Now both are at position 1
    EXPECT_EQ(it1, it2);
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
}

TEST_F(VertexIteratorTest, EmptyGraph) {
    auto empty_graph = std::make_unique<gsp::VertexGraph>(0);
    
    auto begin_it = empty_graph->begin();
    auto end_it = empty_graph->end();
    
    EXPECT_EQ(begin_it, end_it);  // For empty graph, begin should equal end
}

TEST_F(VertexIteratorTest, SingleElementGraph) {
    auto single_graph = std::make_unique<gsp::VertexGraph>(1);
    single_graph->setCoord(0, gsp::Coord(5.0, 5.0, 5.0));
    std::vector<std::string> names = {"single"};
    single_graph->setNames(names);
    
    auto begin_it = single_graph->begin();
    auto end_it = single_graph->end();
    
    ASSERT_NE(begin_it, end_it);  // Should not be equal initially
    
    EXPECT_EQ(begin_it->id, 0);
    EXPECT_EQ(begin_it->name, "single");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 5.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 5.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 5.0);
    
    ++begin_it;
    EXPECT_EQ(begin_it, end_it);  // Should now be at end
}