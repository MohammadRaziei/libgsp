//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/GraphSignal.h"
#include "libgsp/Signal.h"
#include "libgsp/utils/Logging.h"
#include <cmath>


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

    const auto sigcoords_d = sigcoords.apply([](const gsp::Coord& coord)->double
     { return std::sqrt(coord.x * coord.x + coord.y * coord.y + coord.z * coord.z); });
    logger->info("sigcoords_d: {}", sigcoords_d.str());

    gsp::Signal<double> sigi2d;
    sigi2d = sigi2.apply(gsp::function::todouble<int>);
    logger->info("sigi2d: {}", sigi2d.str());

    sigi2d = gsp::arrayfun(sigi2, gsp::function::todouble<int>);
    logger->info("sigi2d: {}", sigi2d.str());


    Eigen::MatrixXd mat{
            {1, 2, 0, 4},   // row0 depends on col0,col1,col3 → all valid
            {0, 1, 0, 1},   // row1 depends on col1,col3      → all valid
            {5, 0, 7, 0},   // row2 touches col2              → invalid
            {0, 0, 9, 10}   // row3 touches col2              → invalid
    };

    logger->info("mat:\n{}", fmt::streamed(mat));
    auto sigd2 = mat * sigd1;
    logger->info("sigd2.mask = {}", sigd1.mask().str());

    logger->info("sigd2 = mat * sigd1: {}", sigd2.str());

    gsp::Signal<double> sigfull = {1, 2, 3, 4};
    logger->info("sigfull: {}", sigfull.str());
    auto sigfull2 = mat * sigfull;
    logger->info("sigfull2 = mat * sigfull: {}", sigfull2.str());
    sigfull.setMask(2, false); // make one element invalid
    logger->info("sigfull: {}", sigfull.str());
    sigfull2 = mat * sigfull;
    logger->info("sigfull2 = mat * sigfull: {}", sigfull2.str());

    gsp::arrayfun(sigfull2, [](double x)->double { return x * 2; });
    logger->info("sigfull2 = {} after arrayfun", sigfull2.str());

    Eigen::MatrixXd mat2({
        {1, 2, 0, 0},
        {0, 1, 0, 1},
        {5, 0, 7, 0}
    });

    logger->info("mat2:\n{}", fmt::streamed(mat2));

    sigfull2 = mat2 * sigfull;
    logger->info("sigfull2 = mat2 * sigfull: {} with size of {}!", sigfull2.str(), sigfull2.size());
    
    sigfull2.applyInplace([](const double x)->double { return x * 2; });
    logger->info("sigfull2 = {} after applyInplace", sigfull2.str());


    return 0;
}