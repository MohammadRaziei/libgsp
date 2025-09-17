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
    fmt::print("sigd1: {}\n", sigd1.str());

    gsp::Signal<int> sigi1;
    fmt::print("sigi1: {}\n", sigi1.str());


    return 0;
}