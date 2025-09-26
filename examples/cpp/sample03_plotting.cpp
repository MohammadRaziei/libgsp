//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/Graph.h"
#include "libgsp/GraphSvg.h"
#include "libgsp/Signal.h"
#include "libgsp/plotting/FigureManager.h"
#include "libgsp/utils/Logging.h"

#include <filesystem>

namespace fs = std::filesystem;

int main() {

    gsp::logging::setDefaultConfigs(gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 03 - Plotting graph with signals");


    gsp::Figure fig1, fig2("test");
    {
        auto& manager = gsp::FigureManager::instance();
        manager.addFigure(fig1);
        manager.addFigure(fig2);
    }
    auto& manager = gsp::FigureManager::instance();
    manager.addFigure(fig2);

    // fig1.setSubplots(2,2);
    // ax = fig1.getSubplot(1);
    // Plot(graph, ax);


    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    gsp::Signal<double>     signal     = {-0.04, 0.31, 0.06, 0.39};

    gsp::DenseGraph graph(4);
    graph.setEdges(edges);
    graph.setCoords(coords_vec);

    gsp::GraphSvg graph_svg(graph, signal);
    std::string svg= graph_svg.svg();

    graph_svg.save((fs::temp_directory_path() / "graph.svg").string());





    // manager.serve(8085);


    return 0;
}