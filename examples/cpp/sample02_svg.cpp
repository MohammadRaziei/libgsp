//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/Graph.h"
#include "libgsp/GraphSvg.h"
#include "libgsp/plotting/FigureManager.h"
#include "libgsp/utils/Logging.h"

#include <filesystem>

namespace fs = std::filesystem;

int main() {

    gsp::logging::setDefaultConfigs(spdlog::level::debug);
    auto logger = gsp::logging::getLogger();

    logger->info("svg test");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    std::vector<double>     signal_vec = {-0.04, 0.31, 0.06, 0.39};

    gsp::DenseGraph graph(4);
    graph.setEdges(edges);
    graph.setCoords(coords_vec);

    gsp::GraphSvg svg(graph);
    const auto path = fs::temp_directory_path() / "graph.svg";
    svg.save(path.string());







    return 0;
}