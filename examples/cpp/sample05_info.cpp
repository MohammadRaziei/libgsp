//
// Created by Mohammad on 9/26/2025.
//

#include <string>
#include "libgsp/utils/GspInfo.h"
#include "libgsp/utils/Logging.h"

int main() {
    gsp::logging::basicConfig(gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    auto& info = gsp::info::GspInfo::instance();

    logger->info("version  : {}", info.version());
    logger->info("language : {}", info.language());
    logger->info("os       : {}", info.os());
    logger->info("author   : {}", info.authorName());
    logger->info("email    : {}", info.authorEmail());
    logger->info("site     : {}", info.siteUrl());

    // ---- list dependencies (git submodules) ----
    const auto& deps = info.dependencies();
    auto typeToStr = [](gsp::info::SubmoduleType t) -> const char* {
        switch (t) {
            case gsp::info::SubmoduleType::Source: return "src";
            case gsp::info::SubmoduleType::Test:   return "test";
            default:                               return "unknown";
        }
    };

    if (deps.empty()) {
        logger->warn("dependencies: none found");
    } else {
        logger->info("dependencies ({}):", deps.size());
        for (const auto& [key, sm] : deps) {
            // key is usually the last component of path; sm.name duplicates that
            logger->info("  {:<16} type={:<7} path={} url={}",
                         sm.name, typeToStr(sm.type), sm.path, sm.url);
        }
    }

    logger->info("summary (verbose is `false` by default):");
    gsp::info::GspInfo::summary();

    logger->info("summary (verbose is `true`):");
    gsp::info::GspInfo::summary(true);
    return 0;
}
