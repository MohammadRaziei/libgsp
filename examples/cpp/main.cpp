//
// Created by mohammad on 5/20/23.
//
#include <iostream>
#include <memory>
#include <filesystem>


#include <spdlog/spdlog.h>

#include <mustache.hpp>
#include <lunasvg.h>

#include "libgsp/graph/graph.h"
#include "libgsp/graph/graphsignal.h"
#include "libgsp/graph/edgegenerator.h"



#include "libgsp/io/file.h"
#include "libgsp/utils/utils.h"
#include "common.h"

namespace fs = std::filesystem;


int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    spdlog::info("Hello, world!");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    std::vector<double>     signal_vec = {-0.04, 0.31, 0.06, 0.39};

    const uint32_t num_nodes = 4;

    // Build graph
    gsp::DenseGraph graph(num_nodes);
    graph.setCoords(coords_vec);
    graph.setEdges(edges);           // fills graph.weights
    graph.setNames({"A", "B", "C", "D"});

    gsp::EdgeGenerator gen(&graph);

    tic;
    uint32_t num_edges = 0;
    while (auto edge = gen.next()) {
        printf("from %d to %d with weight %.2f\n", edge->source, edge->target, edge->weight);
        num_edges++;
    }
    toc;
    std::cout << "num edges = " << num_edges << std::endl;

//
//    tic;
//    for (int i = 0; i < 1'000'000; ++i) {
//        num_edges = 0;
//        gen.iter();
//        while (auto edge = gen.next()) {
//            if (!edge) break;  // no more edges
//            // printf("from %d to %d with weight %.2f\n", edge->source, edge->target, edge->weight);
//            num_edges++;
//        }
//    }
//    toc;
//    std::cout << "num edges = " << num_edges << std::endl;
//
//

    graph.setEdges(edges, false);           // fills graph.weights
    graph.setIsDirectedUnsafe(true);

    num_edges = 0;
    graph.edgeIter();
    while (auto edge = graph.edgeNext()) {
        printf("from %d to %d with weight %.2f\n", edge->source, edge->target, edge->weight);
        num_edges++;
    }
    std::cout << "num edges = " << num_edges << std::endl;

    std::cout << "coords:\n" << graph.coords() << "\n";

    // Shorthands
    const Eigen::MatrixXd& W = graph.weights();      // adjacency (NxN)
    std::cout << "Weights matrix W:\n" << W << "\n";


    // degree = sum of rows
    Eigen::VectorXd degree = graph.degrees();     // degree vector (1xN)
    // L = D - W
    Eigen::MatrixXd L = graph.laplacian();



    // Log/print
    std::cout << "Degree vector:\n" << degree.transpose() << "\n";
    std::cout << "Laplacian matrix L:\n" << L << "\n";

    // Signal (view over std::vector without copy)
    Eigen::Map<const Eigen::VectorXd> signal(signal_vec.data(), num_nodes);
    gsp::GraphSignal graph_signal(&graph, (Eigen::VectorXd)signal);

    std::puts("\ngood bye :)\n");

    // Render SVG (unchanged)
    std::map<std::string, std::string> options = {
        {"signal_scale",     "100"},
        {"node_space_scale", "100"},
        {"signal_font_size", "6"},
        {"title",            "Network"},
    };

    // std::string svg = render_svg(edges, coords_vec, signal_vec, options);
//    writeFile("graph.svg", svg);
    spdlog::info("SVG file written to graph.svg");

//    return svg2png(svg);
}
