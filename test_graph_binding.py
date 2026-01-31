#!/usr/bin/env python3
"""Test script for libgsp graph bindings"""

import sys
import traceback

def test_basic_types():
    """Test basic types from graph module"""
    try:
        # Try to import the graph module
        from libgsp import graph
        
        print("✓ Successfully imported graph module")
        
        # Test basic structs
        print("\nTesting Coord struct:")
        coord = graph.Coord(1.0, 2.0, 3.0)
        print(f"  Created coord: {coord}")
        print(f"  x: {coord.x}, y: {coord.y}, z: {coord.z}")
        
        print("\nTesting Edge struct:")
        edge = graph.Edge(0, 1, 0.5)
        print(f"  Created edge: {edge}")
        print(f"  source: {edge.source}, target: {edge.target}, weight: {edge.weight}")
        
        print("\nTesting Node struct:")
        node = graph.Node(0, coord, "node0")
        print(f"  Created node: {node}")
        print(f"  id: {node.id}, name: {node.name}")
        
        print("\nTesting VertexGraph:")
        vg = graph.VertexGraph(5)
        print(f"  Created VertexGraph with {vg.num_nodes} nodes")
        
        print("\nTesting BaseGraph:")
        # BaseGraph is abstract, can't instantiate directly
        
        print("\nTesting Graph classes:")
        sparse_graph = graph.SparseGraph(5)
        print(f"  Created SparseGraph with {sparse_graph.num_nodes} nodes")
        
        dense_graph = graph.DenseGraph(5)
        print(f"  Created DenseGraph with {dense_graph.num_nodes} nodes")
        
        print("\nTesting Signal classes:")
        signal_double = graph.SignalDouble(10)
        print(f"  Created SignalDouble with size {signal_double.size()}")
        
        signal_float = graph.SignalFloat(5)
        print(f"  Created SignalFloat with size {signal_float.size()}")
        
        print("\nTesting SignalMask:")
        mask = graph.SignalMask(10)
        print(f"  Created SignalMask with size {mask.size()}")
        
        print("\n✓ All basic types tested successfully!")
        return True
        
    except Exception as e:
        print(f"✗ Error during testing: {e}")
        traceback.print_exc()
        return False

def test_graph_operations():
    """Test graph operations"""
    try:
        from libgsp import graph
        
        print("\nTesting graph operations:")
        
        # Create a simple graph
        g = graph.DenseGraph(3)
        
        # Create edges
        edges = [
            graph.Edge(0, 1, 1.0),
            graph.Edge(1, 2, 2.0),
            graph.Edge(2, 0, 3.0)
        ]
        
        # Set edges
        g.setEdges(edges)
        print(f"  Set {len(edges)} edges on graph")
        
        # Get edges
        retrieved_edges = g.edges()
        print(f"  Retrieved {len(retrieved_edges)} edges")
        
        # Test weights
        weights = g.weights()
        print(f"  Got weights matrix with shape ({weights.rows()}, {weights.cols()})")
        
        print("\n✓ Graph operations tested successfully!")
        return True
        
    except Exception as e:
        print(f"✗ Error during graph operations: {e}")
        traceback.print_exc()
        return False

def main():
    """Main test function"""
    print("Testing libgsp graph Python bindings")
    print("=" * 50)
    
    success = True
    
    # Test basic types
    if not test_basic_types():
        success = False
    
    # Test graph operations
    if success:
        if not test_graph_operations():
            success = False
    
    print("\n" + "=" * 50)
    if success:
        print("✓ All tests passed!")
        return 0
    else:
        print("✗ Some tests failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())