//
// Created by mohammad on 9/9/25.
//

#include "libgsp/utils/logging.h"


#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void gsp::logging::setDefaultConfigs(spdlog::level::level_enum level,
                       const std::string& pattern) {
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("app.log", true);

    auto root = std::make_shared<spdlog::logger>("root",
                  spdlog::sinks_init_list{console, file});
    spdlog::set_default_logger(root);

    spdlog::set_pattern(pattern);
    spdlog::set_level(level);
}


std::shared_ptr<spdlog::logger> gsp::logging::getLogger(const std::string& name) {
    if (name.empty() || name == "root") {
        return spdlog::default_logger();
    }

    auto lg = spdlog::get(name);
    if (!lg) {
        lg = spdlog::default_logger()->clone(name);
        spdlog::register_logger(lg);
    }
    return lg;
}