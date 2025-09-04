//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"


int main() {
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
    manager.serve();

    return 0;
}