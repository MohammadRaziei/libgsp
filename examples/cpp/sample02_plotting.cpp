//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"
#include "libgsp/utils/logging.h"

int main() {

    gsp::logging::setDefaultConfigs(spdlog::level::debug);
    auto logger = gsp::logging::getLogger();

    logger->info("Plotting test");

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
    manager.serve(8085);


    return 0;
}