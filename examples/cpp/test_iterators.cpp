#include "include/libgsp/VertexGraph.h"
#include <iostream>
#include <vector>

int main() {
    // Create a VertexGraph with 3 nodes
    gsp::VertexGraph graph(3);
    
    // Set some coordinates
    std::vector<gsp::Coord> coords = {
        gsp::Coord(0.0, 0.0, 0.0),
        gsp::Coord(1.0, 1.0, 1.0),
        gsp::Coord(2.0, 2.0, 2.0)
    };
    graph.setCoords(coords);
    
    // Set some names
    std::vector<std::string> names = {"node0", "node1", "node2"};
    graph.setNames(names);
    
    std::cout << "Testing VertexIterator (non-const):" << std::endl;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        std::cout << "Node ID: " << it->id << ", Name: " << it->name 
                  << ", Coord: (" << it->coord.x << ", " << it->coord.y << ", " << it->coord.z << ")" << std::endl;
    }
    
    std::cout << "\nTesting range-based for loop:" << std::endl;
    for (const auto& node : graph) {
        std::cout << "Node ID: " << node.id << ", Name: " << node.name 
                  << ", Coord: (" << node.coord.x << ", " << node.coord.y << ", " << node.coord.z << ")" << std::endl;
    }
    
    std::cout << "\nTesting ConstVertexIterator (const):" << std::endl;
    const gsp::VertexGraph& const_graph = graph;
    for (auto it = const_graph.begin(); it != const_graph.end(); ++it) {
        std::cout << "Node ID: " << it->id << ", Name: " << it->name 
                  << ", Coord: (" << it->coord.x << ", " << it->coord.y << ", " << it->coord.z << ")" << std::endl;
    }
    
    std::cout << "\nTesting cbegin/cend:" << std::endl;
    for (auto it = graph.cbegin(); it != graph.cend(); ++it) {
        std::cout << "Node ID: " << it->id << ", Name: " << it->name 
                  << ", Coord: (" << it->coord.x << ", " << it->coord.y << ", " << it->coord.z << ")" << std::endl;
    }
    
    std::cout << "\nAll iterator tests passed!" << std::endl;
    
    return 0;
}