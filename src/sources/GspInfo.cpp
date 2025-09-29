#include "libgsp/utils/GspInfo.h"

#include <regex>
#include <string>

#include <toml++/toml.h>

#include "project/pyproject_toml.h"
#include "project/gitmodules.h"


namespace gsp::info {

GspInfo& GspInfo::instance() {
    static GspInfo inst;
    return inst;
}

void GspInfo::summary(bool verbose) {
    fmt::print(instance().str(verbose));
}

GspInfo::GspInfo()
    : _logger(gsp::logging::getLogger("GSPInfo"))
    , _language("cpp")
    , _os(GspInfo::detectOs()) {
    // defaults
    _name = "libgsp";
    _version = "__dev__";

    parsePyproject();
    parseGitmodules();
}

void GspInfo::parsePyproject() {
    // ---- load project metadata from pyproject.toml ----
    try {
        toml::table root = toml::parse(project::pyproject_toml);

        if (auto proj = root["project"].as_table()) {
            if (auto v = (*proj)["name"].value<std::string>())    _name = *v;
            if (auto v = (*proj)["version"].value<std::string>()) _version = *v;

            if (auto arr = (*proj)["authors"].as_array()) {
                if (!arr->empty()) {
                    if (auto tbl = (*arr)[0].as_table()) {
                        if (auto v = (*tbl)["name"].value<std::string>())  _author_name = *v;
                        if (auto v = (*tbl)["email"].value<std::string>()) _author_email = *v;
                    }
                }
            }

            // read from [project.urls].Document
            if (auto urls = (*proj)["urls"].as_table()) {
                if (auto v = (*urls)["Document"].value<std::string>()) {
                    _site_url = *v;
                } else {
                    _logger->error("Missing 'Document' in [project.urls].");
                }
            } else {
                _logger->error("Missing [project.urls] table in pyproject.toml.");
            }
        } else {
            _logger->error("Missing [project] table in pyproject.toml.");
        }
    } catch (const toml::parse_error& e) {
        _logger->error("Failed to parse pyproject.toml: {}", e.description());
        // keep defaults
    }
}
void GspInfo::parseGitmodules() {
    _deps.clear();

    // 1) Rewrite headers: [submodule "path/to/name"] -> [submodule.name]
    static const std::regex header_re(R"regex(\[\s*submodule\s+"(?:.*/)?([^"/\]]+)"\s*\])regex");
    std::string gitmodules = std::regex_replace(project::gitmodules, header_re, "[submodule.$1]");

    // 2) Quote values for TOML validity on common string keys (path/url/branch)
    static const std::regex need_quote_re(
        R"regex((^|\n)(\s*(?:path|url|branch)\s*=\s*)([^"\r\n][^\r\n]*))regex"
    );
    gitmodules = std::regex_replace(gitmodules, need_quote_re, "$1$2\"$3\"");

    // 3) Parse as TOML using toml++
    try {
        toml::table mods = toml::parse(gitmodules);

        auto classifyPath = [](const std::string& p) {
            if (p.rfind("src/",   0) == 0) return SubmoduleType::Source;
            if (p.rfind("tests/", 0) == 0) return SubmoduleType::Test;
            return SubmoduleType::Unknown;
        };

        // After rewrite, submodules live under the [submodule] table
        if (auto subs = mods["submodule"].as_table()) {
            for (auto&& [name_key, node] : *subs) {
                const std::string name = std::string(name_key.str());
                if (auto tbl = node.as_table()) {
                    Submodule sm;
                    sm.name = name;
                    sm.path = (*tbl)["path"].value_or(std::string{});
                    sm.url  = (*tbl)["url"].value_or(std::string{});
                    sm.type = classifyPath(sm.path);

                    if (!sm.name.empty())
                        _deps[sm.name] = std::move(sm);
                }
            }
        }
    } catch (const toml::parse_error& e) {
        _logger->error("Failed to parse rewritten .gitmodules as TOML: {}", e.description());
    }
}

std::string GspInfo::detectOs() {
#if defined(_WIN32)
    return "windows";
#elif defined(__APPLE__) && defined(__MACH__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#elif defined(__ANDROID__)
    return "android";
#elif defined(__unix__)
    return "unix";
#elif defined(__EMSCRIPTEN__)
    return "emscripten";
#else
    return "unknown";
#endif
}

std::string GspInfo::str(bool verbose) const {
    auto trunc = [](std::string s, std::size_t maxlen) -> std::string {
        if (s.size() <= maxlen) return s;
        if (maxlen <= 3) return s.substr(0, maxlen);
        return s.substr(0, maxlen - 3) + "...";
    };

    auto type_icon = [](SubmoduleType t) -> char {
        switch (t) {
            case SubmoduleType::Source:  return 'S';
            case SubmoduleType::Test:    return 'T';
            default:                     return '?';
        }
    };

    constexpr int width = 100;
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "╔{0:═^{1}}╗\n", " G S P  —  Project Info ", width);

    std::string _fmt = std::string("║ {0: <16}: {1: <") + std::to_string(width-20) +"} ║\n";
    fmt::format_to(std::back_inserter(buf), _fmt, "Name", trunc(_name, 57));
    fmt::format_to(std::back_inserter(buf), _fmt, "Version", trunc(_version, 57));
    fmt::format_to(std::back_inserter(buf), _fmt, "Language", trunc(_language, 57));
    fmt::format_to(std::back_inserter(buf), _fmt, "OS", trunc(_os, 57));
    fmt::format_to(std::back_inserter(buf), _fmt, "Author",
                   fmt::format("{} <{}>", trunc(_author_name, 38),
                               trunc(_author_email, width - 38 - 5)));
    fmt::format_to(std::back_inserter(buf), _fmt, "Website", trunc(_site_url, 57));

    if (verbose) {
        fmt::format_to(std::back_inserter(buf), "╠{0:═^{1}}╣\n",
                       fmt::format(" Dependencies ({}) ", _deps.size()), width);

        const int url_width = width - 56;
        _fmt = std::string("║ {0:>3} │ {1:<18} │ {2:<20} │ {3:<") +
               std::to_string(url_width) + "} │ {4} ║\n";
        fmt::format_to(std::back_inserter(buf), _fmt, "Num", "Name", "Path", "URL", "X");
        fmt::format_to(std::back_inserter(buf), "╟{0:─^{1}}╢\n", "", width);

        if (_deps.empty()) {
            fmt::format_to(std::back_inserter(buf), "║ {0: ^{1}} ║\n",
                           "— no dependencies —", width);
        } else {
            int i = 0;
            for (const auto& kv : _deps) {
                const auto& sm = kv.second;
                fmt::format_to(std::back_inserter(buf), _fmt, ++i,
                               trunc(sm.name, 18),
                               trunc(sm.path, 20),
                               trunc(sm.url, url_width),
                               type_icon(sm.type));
            }
        }
    }

    fmt::format_to(std::back_inserter(buf), "╚{0:═^{1}}╝\n", "", width);

    return fmt::to_string(buf);
}



} // namespace gsp::info
