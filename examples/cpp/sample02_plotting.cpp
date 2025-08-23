//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"


int main() {
    auto& manager = FigureManager::instance();
    Figure fig1, fig2("test");
    manager.addFigure(fig1);
    manager.addFigure(fig2);
    manager.serve();

    return 0;
}