//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/GraphSignal.h"
#include "libgsp/utils/Logging.h"


int main() {

    gsp::logging::setDefaultConfigs(spdlog::level::debug);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 04: Graph Signals");

    gsp::Signal<double> sigd1({1, 2, 3, 4});
    sigd1.set(1, std::nullopt);
    sigd1.set(2, 5);
    logger->info("sigd1: {}", sigd1.str());

    gsp::Signal<int> sigi1({1, 2, 3, 4}, {1, 1, 0, 1});
    logger->info("sigi1: {}", sigi1.str());

    gsp::Signal<int> sigi2({1, 2, 3, 4}, {{2, 0}, {1, 1}});
    logger->info("sigi2: {}", sigi1.str());

    const auto vec = sigi2.vector();

    gsp::Signal<gsp::Coord> sigcoords({{1, 1, 0}, {2, 1, 0}, {3, 3, 0}, {4,4, 0}},
        {{2, 0}, {1, 1}});

    int i = 0;
    for (const auto coord : sigcoords.vector()) {
        logger->info("coord {}: {}, {}, {}", i++, coord->x, coord->y, coord->z);
    }
    return 0;
}