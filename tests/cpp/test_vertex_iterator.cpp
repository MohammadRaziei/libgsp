//
// Created by Mohammad on 1/3/2026.
//

#include <gtest/gtest.h>
#include <vector>

#include "libgsp/VertexGraph.h"

class VertexIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<gsp::VertexGraph>(5);

        // Set coordinates and names for the nodes
        graph->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        graph->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        graph->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));
        graph->setCoord(3, gsp::Coord(3.0, 3.0, 3.0));
        graph->setCoord(4, gsp::Coord(4.0, 4.0, 4.0));

        std::vector<std::string> names = {"node0", "node1", "node2", "node3", "node4"};
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

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 2);
    EXPECT_EQ(begin_it->name, "node2");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 2.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 3);
    EXPECT_EQ(begin_it->name, "node3");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 3.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 4);
    EXPECT_EQ(begin_it->name, "node4");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 4.0);

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

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 2);
    EXPECT_EQ(begin_it->name, "node2");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 2.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 3);
    EXPECT_EQ(begin_it->name, "node3");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 3.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 4);
    EXPECT_EQ(begin_it->name, "node4");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 4.0);

    // Test increment to end
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

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 2);
    EXPECT_EQ(begin_it->name, "node2");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 2.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 2.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 3);
    EXPECT_EQ(begin_it->name, "node3");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 3.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 3.0);

    // Test increment
    ++begin_it;
    ASSERT_NE(begin_it, end_it);  // Should not be at end yet
    EXPECT_EQ(begin_it->id, 4);
    EXPECT_EQ(begin_it->name, "node4");
    EXPECT_DOUBLE_EQ(begin_it->coord.x(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.y(), 4.0);
    EXPECT_DOUBLE_EQ(begin_it->coord.z(), 4.0);

    // Test increment to end
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

    EXPECT_EQ(nodes.size(), 5);
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

    EXPECT_EQ(nodes[3].id, 3);
    EXPECT_EQ(nodes[3].name, "node3");
    EXPECT_DOUBLE_EQ(nodes[3].coord.x(), 3.0);
    EXPECT_DOUBLE_EQ(nodes[3].coord.y(), 3.0);
    EXPECT_DOUBLE_EQ(nodes[3].coord.z(), 3.0);

    EXPECT_EQ(nodes[4].id, 4);
    EXPECT_EQ(nodes[4].name, "node4");
    EXPECT_DOUBLE_EQ(nodes[4].coord.x(), 4.0);
    EXPECT_DOUBLE_EQ(nodes[4].coord.y(), 4.0);
    EXPECT_DOUBLE_EQ(nodes[4].coord.z(), 4.0);
}

TEST_F(VertexIteratorTest, ConstRangeBasedForLoop) {
    std::vector<gsp::Node> nodes;
    const gsp::VertexGraph& const_graph = *graph;

    // Test range-based for with const
    for (const auto& node : const_graph) {
        nodes.push_back(node);
    }

    EXPECT_EQ(nodes.size(), 5);
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

    EXPECT_EQ(nodes[3].id, 3);
    EXPECT_EQ(nodes[3].name, "node3");
    EXPECT_DOUBLE_EQ(nodes[3].coord.x(), 3.0);
    EXPECT_DOUBLE_EQ(nodes[3].coord.y(), 3.0);
    EXPECT_DOUBLE_EQ(nodes[3].coord.z(), 3.0);

    EXPECT_EQ(nodes[4].id, 4);
    EXPECT_EQ(nodes[4].name, "node4");
    EXPECT_DOUBLE_EQ(nodes[4].coord.x(), 4.0);
    EXPECT_DOUBLE_EQ(nodes[4].coord.y(), 4.0);
    EXPECT_DOUBLE_EQ(nodes[4].coord.z(), 4.0);
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

// Additional tests for enhanced iterator functionality
TEST_F(VertexIteratorTest, EnhancedIteratorArithmeticPlusEquals) {
    auto it = graph->begin();

    // Test += operator
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");

    it += 1;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "node1");

    it += 1;
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");
}

TEST_F(VertexIteratorTest, EnhancedIteratorArithmeticMinusEquals) {
    auto it = graph->begin() + 2;

    // Test -= operator
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");

    it -= 1;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "node1");

    it -= 1;
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(VertexIteratorTest, EnhancedIteratorArithmeticPlusOperator) {
    auto it = graph->begin();

    // Test + operator
    auto it_plus_2 = it + 2;
    EXPECT_EQ(it_plus_2->id, 2);
    EXPECT_EQ(it_plus_2->name, "node2");

    auto it_plus_1 = it + 1;
    EXPECT_EQ(it_plus_1->id, 1);
    EXPECT_EQ(it_plus_1->name, "node1");

    // Original iterator should not be modified
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(VertexIteratorTest, EnhancedIteratorArithmeticMinusOperator) {
    auto it = graph->begin() + 2;

    // Test - operator
    auto it_minus_1 = it - 1;
    EXPECT_EQ(it_minus_1->id, 1);
    EXPECT_EQ(it_minus_1->name, "node1");

    auto it_minus_2 = it - 2;
    EXPECT_EQ(it_minus_2->id, 0);
    EXPECT_EQ(it_minus_2->name, "node0");

    // Original iterator should not be modified
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");
}

TEST_F(VertexIteratorTest, EnhancedIteratorSubtraction) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;

    // Test subtraction between iterators
    auto distance = it2 - it1;
    EXPECT_EQ(distance, 2);

    auto distance2 = it1 - it2;
    EXPECT_EQ(distance2, -2);

    // Same iterator should have distance 0
    auto distance3 = it1 - it1;
    EXPECT_EQ(distance3, 0);
}

TEST_F(VertexIteratorTest, EnhancedIteratorSubscriptOperator) {
    auto it = graph->begin();

    // Test subscript operator
    EXPECT_EQ(it[0].id, 0);
    EXPECT_EQ(it[0].name, "node0");

    EXPECT_EQ(it[1].id, 1);
    EXPECT_EQ(it[1].name, "node1");

    EXPECT_EQ(it[2].id, 2);
    EXPECT_EQ(it[2].name, "node2");

    // Original iterator should not be modified
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(VertexIteratorTest, EnhancedIteratorSubscriptOperatorConst) {
    const gsp::VertexGraph& const_graph = *graph;
    auto it = const_graph.begin();

    // Test subscript operator with const iterator
    EXPECT_EQ(it[0].id, 0);
    EXPECT_EQ(it[0].name, "node0");

    EXPECT_EQ(it[3].id, 3);
    EXPECT_EQ(it[3].name, "node3");
}

TEST_F(VertexIteratorTest, EnhancedIteratorLessThanComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 1;
    auto it3 = graph->begin() + 2;

    // Test < operator
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it2 < it3);
    EXPECT_TRUE(it1 < it3);

    EXPECT_FALSE(it2 < it1);
    EXPECT_FALSE(it3 < it2);
    EXPECT_FALSE(it3 < it1);

    // Self comparison should be false
    EXPECT_FALSE(it1 < it1);
}

TEST_F(VertexIteratorTest, EnhancedIteratorLessThanOrEqualComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;

    // Test <= operator
    EXPECT_TRUE(it1 <= it2);
    EXPECT_TRUE(it1 <= it1);  // Self comparison
    EXPECT_FALSE(it2 <= it1);

    auto it3 = graph->begin() + 2;
    EXPECT_TRUE(it2 <= it3);  // Equal iterators
}

TEST_F(VertexIteratorTest, EnhancedIteratorGreaterThanComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 1;
    auto it3 = graph->begin() + 2;

    // Test > operator
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it3 > it2);
    EXPECT_TRUE(it3 > it1);

    EXPECT_FALSE(it1 > it2);
    EXPECT_FALSE(it2 > it3);
    EXPECT_FALSE(it1 > it3);

    // Self comparison should be false
    EXPECT_FALSE(it1 > it1);
}

TEST_F(VertexIteratorTest, EnhancedIteratorGreaterThanOrEqualComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;

    // Test >= operator
    EXPECT_TRUE(it2 >= it1);
    EXPECT_TRUE(it1 >= it1);  // Self comparison
    EXPECT_FALSE(it1 >= it2);

    auto it3 = graph->begin() + 2;
    EXPECT_TRUE(it2 >= it3);  // Equal iterators
}

TEST_F(VertexIteratorTest, EnhancedIteratorRandomAccessIteration) {
    // Test random access iteration
    std::vector<gsp::Node> nodes;
    for (int i = 0; i < 3; ++i) {  // Use smaller range for our test graph
        auto it = graph->begin() + i;
        nodes.push_back(*it);
    }

    EXPECT_EQ(nodes.size(), 3);
    EXPECT_EQ(nodes[0].id, 0);
    EXPECT_EQ(nodes[0].name, "node0");
    EXPECT_EQ(nodes[1].id, 1);
    EXPECT_EQ(nodes[1].name, "node1");
    EXPECT_EQ(nodes[2].id, 2);
    EXPECT_EQ(nodes[2].name, "node2");
}

TEST_F(VertexIteratorTest, EnhancedIteratorConstIteratorArithmetic) {
    const gsp::VertexGraph& const_graph = *graph;
    auto it = const_graph.begin();

    // Test arithmetic operations with const iterator
    auto it_plus_2 = it + 2;
    EXPECT_EQ(it_plus_2->id, 2);
    EXPECT_EQ(it_plus_2->name, "node2");

    auto it_plus_1 = it_plus_2 - 1;
    EXPECT_EQ(it_plus_1->id, 1);
    EXPECT_EQ(it_plus_1->name, "node1");

    EXPECT_EQ((it_plus_2 - it), 2);
    EXPECT_TRUE(it < it_plus_2);
    EXPECT_TRUE(it_plus_2 > it);
}

TEST_F(VertexIteratorTest, EnhancedIteratorDistanceCalculation) {
    auto begin_it = graph->begin();
    auto end_it = graph->end();

    // Calculate total distance
    auto total_distance = end_it - begin_it;
    EXPECT_EQ(total_distance, 5);  // Our test graph has 5 nodes

    // Test partial distances
    auto mid_it = begin_it + 2;
    EXPECT_EQ((mid_it - begin_it), 2);
    EXPECT_EQ((end_it - mid_it), 3);
}

TEST_F(VertexIteratorTest, EnhancedIteratorOutOfRangeSubscript) {
    auto it = graph->begin();

    // Test that out of range access throws exception
    EXPECT_THROW(it[10], std::out_of_range);
    EXPECT_THROW((it + 5)[0], std::out_of_range);
    EXPECT_THROW((it - 1)[0], std::out_of_range);
}

TEST_F(VertexIteratorTest, EnhancedIteratorBidirectionalCompatibility) {
    // Ensure backward compatibility with bidirectional operations
    auto it = graph->begin();

    // Test increment
    ++it;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "node1");

    // Test decrement
    --it;
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");

    // Test post-increment
    auto old_it = it++;
    EXPECT_EQ(old_it->id, 0);
    EXPECT_EQ(it->id, 1);

    // Test post-decrement
    auto old_it2 = it--;
    EXPECT_EQ(old_it2->id, 1);
    EXPECT_EQ(it->id, 0);
}



class EnhancedIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<gsp::VertexGraph>(5);
        
        // Set coordinates and names for the nodes
        graph->setCoord(0, gsp::Coord(0.0, 0.0, 0.0));
        graph->setCoord(1, gsp::Coord(1.0, 1.0, 1.0));
        graph->setCoord(2, gsp::Coord(2.0, 2.0, 2.0));
        graph->setCoord(3, gsp::Coord(3.0, 3.0, 3.0));
        graph->setCoord(4, gsp::Coord(4.0, 4.0, 4.0));
        
        std::vector<std::string> names = {"node0", "node1", "node2", "node3", "node4"};
        graph->setNames(names);
    }

    std::unique_ptr<gsp::VertexGraph> graph;
};

TEST_F(EnhancedIteratorTest, ArithmeticPlusEquals) {
    auto it = graph->begin();
    
    // Test += operator
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
    
    it += 2;
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");
    
    it += 1;
    EXPECT_EQ(it->id, 3);
    EXPECT_EQ(it->name, "node3");
}

TEST_F(EnhancedIteratorTest, ArithmeticMinusEquals) {
    auto it = graph->begin() + 3;
    
    // Test -= operator
    EXPECT_EQ(it->id, 3);
    EXPECT_EQ(it->name, "node3");
    
    it -= 1;
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");
    
    it -= 2;
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(EnhancedIteratorTest, ArithmeticPlusOperator) {
    auto it = graph->begin();
    
    // Test + operator
    auto it_plus_2 = it + 2;
    EXPECT_EQ(it_plus_2->id, 2);
    EXPECT_EQ(it_plus_2->name, "node2");
    
    auto it_plus_4 = it + 4;
    EXPECT_EQ(it_plus_4->id, 4);
    EXPECT_EQ(it_plus_4->name, "node4");
    
    // Original iterator should not be modified
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(EnhancedIteratorTest, ArithmeticMinusOperator) {
    auto it = graph->begin() + 4;
    
    // Test - operator
    auto it_minus_2 = it - 2;
    EXPECT_EQ(it_minus_2->id, 2);
    EXPECT_EQ(it_minus_2->name, "node2");
    
    auto it_minus_4 = it - 4;
    EXPECT_EQ(it_minus_4->id, 0);
    EXPECT_EQ(it_minus_4->name, "node0");
    
    // Original iterator should not be modified
    EXPECT_EQ(it->id, 4);
    EXPECT_EQ(it->name, "node4");
}

TEST_F(EnhancedIteratorTest, IteratorSubtraction) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 3;
    
    // Test subtraction between iterators
    auto distance = it2 - it1;
    EXPECT_EQ(distance, 3);
    
    auto distance2 = it1 - it2;
    EXPECT_EQ(distance2, -3);
    
    // Same iterator should have distance 0
    auto distance3 = it1 - it1;
    EXPECT_EQ(distance3, 0);
}

TEST_F(EnhancedIteratorTest, SubscriptOperator) {
    auto it = graph->begin();
    
    // Test subscript operator
    EXPECT_EQ(it[0].id, 0);
    EXPECT_EQ(it[0].name, "node0");
    
    EXPECT_EQ(it[1].id, 1);
    EXPECT_EQ(it[1].name, "node1");
    
    EXPECT_EQ(it[2].id, 2);
    EXPECT_EQ(it[2].name, "node2");
    
    EXPECT_EQ(it[4].id, 4);
    EXPECT_EQ(it[4].name, "node4");
    
    // Original iterator should not be modified
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
}

TEST_F(EnhancedIteratorTest, SubscriptOperatorConst) {
    const gsp::VertexGraph& const_graph = *graph;
    auto it = const_graph.begin();
    
    // Test subscript operator with const iterator
    EXPECT_EQ(it[0].id, 0);
    EXPECT_EQ(it[0].name, "node0");
    
    EXPECT_EQ(it[3].id, 3);
    EXPECT_EQ(it[3].name, "node3");
}

TEST_F(EnhancedIteratorTest, LessThanComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;
    auto it3 = graph->begin() + 4;
    
    // Test < operator
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it2 < it3);
    EXPECT_TRUE(it1 < it3);
    
    EXPECT_FALSE(it2 < it1);
    EXPECT_FALSE(it3 < it2);
    EXPECT_FALSE(it3 < it1);
    
    // Self comparison should be false
    EXPECT_FALSE(it1 < it1);
}

TEST_F(EnhancedIteratorTest, LessThanOrEqualComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;
    
    // Test <= operator
    EXPECT_TRUE(it1 <= it2);
    EXPECT_TRUE(it1 <= it1);  // Self comparison
    EXPECT_FALSE(it2 <= it1);
    
    auto it3 = graph->begin() + 2;
    EXPECT_TRUE(it2 <= it3);  // Equal iterators
}

TEST_F(EnhancedIteratorTest, GreaterThanComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;
    auto it3 = graph->begin() + 4;
    
    // Test > operator
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it3 > it2);
    EXPECT_TRUE(it3 > it1);
    
    EXPECT_FALSE(it1 > it2);
    EXPECT_FALSE(it2 > it3);
    EXPECT_FALSE(it1 > it3);
    
    // Self comparison should be false
    EXPECT_FALSE(it1 > it1);
}

TEST_F(EnhancedIteratorTest, GreaterThanOrEqualComparison) {
    auto it1 = graph->begin();
    auto it2 = graph->begin() + 2;
    
    // Test >= operator
    EXPECT_TRUE(it2 >= it1);
    EXPECT_TRUE(it1 >= it1);  // Self comparison
    EXPECT_FALSE(it1 >= it2);
    
    auto it3 = graph->begin() + 2;
    EXPECT_TRUE(it2 >= it3);  // Equal iterators
}

TEST_F(EnhancedIteratorTest, RandomAccessIteration) {
    // Test random access iteration
    std::vector<gsp::Node> nodes;
    for (int i = 0; i < 5; ++i) {
        auto it = graph->begin() + i;
        nodes.push_back(*it);
    }
    
    EXPECT_EQ(nodes.size(), 5);
    EXPECT_EQ(nodes[0].id, 0);
    EXPECT_EQ(nodes[0].name, "node0");
    EXPECT_EQ(nodes[1].id, 1);
    EXPECT_EQ(nodes[1].name, "node1");
    EXPECT_EQ(nodes[2].id, 2);
    EXPECT_EQ(nodes[2].name, "node2");
    EXPECT_EQ(nodes[3].id, 3);
    EXPECT_EQ(nodes[3].name, "node3");
    EXPECT_EQ(nodes[4].id, 4);
    EXPECT_EQ(nodes[4].name, "node4");
}

TEST_F(EnhancedIteratorTest, ConstIteratorArithmetic) {
    const gsp::VertexGraph& const_graph = *graph;
    auto it = const_graph.begin();
    
    // Test arithmetic operations with const iterator
    auto it_plus_3 = it + 3;
    EXPECT_EQ(it_plus_3->id, 3);
    EXPECT_EQ(it_plus_3->name, "node3");
    
    auto it_plus_1 = it_plus_3 - 2;
    EXPECT_EQ(it_plus_1->id, 1);
    EXPECT_EQ(it_plus_1->name, "node1");
    
    EXPECT_EQ((it_plus_3 - it), 3);
    EXPECT_TRUE(it < it_plus_3);
    EXPECT_TRUE(it_plus_3 > it);
}

TEST_F(EnhancedIteratorTest, IteratorDistanceCalculation) {
    auto begin_it = graph->begin();
    auto end_it = graph->end();
    
    // Calculate total distance
    auto total_distance = end_it - begin_it;
    EXPECT_EQ(total_distance, 5);
    
    // Test partial distances
    auto mid_it = begin_it + 3;
    EXPECT_EQ((mid_it - begin_it), 3);
    EXPECT_EQ((end_it - mid_it), 2);
}

TEST_F(EnhancedIteratorTest, OutOfRangeSubscript) {
    auto it = graph->begin();
    
    // Test that out of range access throws exception
    EXPECT_THROW(it[10], std::out_of_range);
    EXPECT_THROW((it + 5)[0], std::out_of_range);
    EXPECT_THROW((it - 1)[0], std::out_of_range);
}

TEST_F(EnhancedIteratorTest, VertexIteratorSpecificFeatures) {
    // Test VertexIterator-specific features
    auto it = graph->begin();
    
    // Test conversion to ConstVertexIterator
    gsp::ConstVertexIterator const_it = it;
    EXPECT_EQ(const_it->id, 0);
    EXPECT_EQ(const_it->name, "node0");
    
    // Test arithmetic operations
    it += 2;
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "node2");
    
    it -= 1;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "node1");
    
    // Test subscript with VertexIterator
    EXPECT_EQ(it[1].id, 2);  // it is at position 1, so it[1] is position 2
    EXPECT_EQ((it + 2)->id, 3);  // it is at position 1, so it+2 is position 3
}

TEST_F(EnhancedIteratorTest, BidirectionalCompatibility) {
    // Ensure backward compatibility with bidirectional operations
    auto it = graph->begin();
    
    // Test increment
    ++it;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "node1");
    
    // Test decrement
    --it;
    EXPECT_EQ(it->id, 0);
    EXPECT_EQ(it->name, "node0");
    
    // Test post-increment
    auto old_it = it++;
    EXPECT_EQ(old_it->id, 0);
    EXPECT_EQ(it->id, 1);
    
    // Test post-decrement
    auto old_it2 = it--;
    EXPECT_EQ(old_it2->id, 1);
    EXPECT_EQ(it->id, 0);
}

TEST_F(EnhancedIteratorTest, EmptyGraphHandling) {
    auto empty_graph = std::make_unique<gsp::VertexGraph>(0);
    
    auto begin_it = empty_graph->begin();
    auto end_it = empty_graph->end();
    
    // For empty graph, begin should equal end
    EXPECT_EQ(begin_it, end_it);
    
    // Operations on empty graph should be safe
    EXPECT_EQ((end_it - begin_it), 0);
    
    // Attempting to access elements should throw
    EXPECT_THROW(*begin_it, std::out_of_range);
    EXPECT_THROW(begin_it[0], std::out_of_range);
}