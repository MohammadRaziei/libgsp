//
// Created by mohammad on 9/9/25.
//

#include "libgsp/utils/Logging.h"

#include <filesystem>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fmt/fmt.h>

namespace fs = std::filesystem;

void gsp::logging::basicConfig(spdlog::level::level_enum level,
                       const std::string& pattern) {
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("app.log", true);

    auto root = std::make_shared<spdlog::logger>("root",
                  spdlog::sinks_init_list{console, file});
    spdlog::set_default_logger(root);

    spdlog::set_pattern(pattern);
    spdlog::set_level(level);
}

namespace gsp::logging {

// Parse "trace|debug|info|warn|error|critical|off"
    static inline spdlog::level::level_enum parse_level_(const char *s) {
        if (!s) return spdlog::level::info;
        if (std::strcmp(s, "trace") == 0) return spdlog::level::trace;
        if (std::strcmp(s, "debug") == 0) return spdlog::level::debug;
        if (std::strcmp(s, "info") == 0) return spdlog::level::info;
        if (std::strcmp(s, "warn") == 0) return spdlog::level::warn;
        if (std::strcmp(s, "error") == 0) return spdlog::level::err;
        if (std::strcmp(s, "critical") == 0) return spdlog::level::critical;
        if (std::strcmp(s, "off") == 0) return spdlog::level::off;
        return spdlog::level::info;
    }


    inline void print_help_(const char *argv0) {
        std::string exe = "app";
        if (argv0) {
            std::filesystem::path p(argv0);
            exe = p.filename().string();
        }

        fmt::print(
                "Usage:\n"
                "  {0} [options]\n"
                "\n"
                "Logging options:\n"
                "  --log-level <trace|debug|info|warn|error|critical|off>\n"
                "      Set global log verbosity (default: info)\n"
                "\n"
                "  --log-pattern \"<pattern>\"\n"
                "      Set spdlog pattern\n"
                "      (default: \"[%Y-%m-%d %H:%M:%S] [%^%l%$] %v\")\n"
                "\n"
                "  --log-file \"<file>\"\n"
                "      Set log file path (default: {0}.log)\n"
                "\n"
                "  -h, --help\n"
                "      Show this help and exit\n"
                "\n"
                "Examples:\n"
                "  {0} --log-level debug\n"
                "  {0} --log-file mylog.log --log-pattern \"[%H:%M:%S] %v\"\n",
                exe
        );
    }

}
// Supported CLI flags:
//   --log-level   <trace|debug|info|warn|error|critical|off>
//   --log-pattern "<pattern>"
//   --log-file    "<file.log>"
void gsp::logging::basicConfig(
    int argc, char** argv,
    gsp::logging::level level, std::string pattern
) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_help_((argc > 0 && argv) ? argv[0] : nullptr);
            exit(0);
        }
    }
    // Defaults derived from argv[0]
    std::string logger_name = "app";
    std::string logfile = "app.log";

    if (argc > 0 && argv && argv[0]) {
        std::filesystem::path exe(argv[0]);
        logger_name = exe.stem().string();
        logfile     = logger_name + ".log";
    }

    // Override defaults via CLI
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            level = parse_level_(argv[++i]);
        } else if (std::strcmp(argv[i], "--log-pattern") == 0 && i + 1 < argc) {
            pattern = argv[++i];
        } else if (std::strcmp(argv[i], "--log-file") == 0 && i + 1 < argc) {
            logfile = argv[++i];
        }
    }

    // If called multiple times, avoid duplicate default loggers
    spdlog::drop_all();

    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file    = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile, true);

    auto logger = std::make_shared<spdlog::logger>(
        logger_name,
        spdlog::sinks_init_list{console, file}
    );

    spdlog::set_default_logger(logger);
    spdlog::set_pattern(pattern);
    spdlog::set_level(level);
}



gsp::logging::Logger gsp::logging::getLogger(const std::string& name) {
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

gsp::logging::Logger gsp::logging::getLoggerByPath(const std::string& path) {
    const std::string fname = fs::path(path).stem().string();
    return getLogger(fname);
}

