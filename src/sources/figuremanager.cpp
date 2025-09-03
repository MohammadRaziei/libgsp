//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"

#include <iostream>
#include <map>
#include <memory>
#include <filesystem>
#include <sstream>
#include <thread>
#include <fstream>
#include <cassert>
#include <mutex>

#include "mustache.hpp"           // for Mustache templating
#include "fmt/fmt.h"              // for string formatting

#ifdef __linux__
#include "httplib.h"              // cpp-httplib (header-only)
#endif

#include "libgsp/utils/string.h"  // for trim()

#include "templates/index_mustache_html.h"

namespace fs = std::filesystem;
using namespace kainjow;

namespace gsp {

// Forward declarations for internal helper functions
static std::string createHtmlIndex(const FigureManager& mgr);

// ---------------- FigureManager Implementation ----------------

/**
 * Constructor: initializes the manager with a given name.
 */
FigureManager::FigureManager(const std::string& name) : _counter(0), _name(name) {
    _figures.reserve(4);
}

/**
 * Returns the default global instance of FigureManager.
 */
FigureManager& FigureManager::defaultInstance() {
    static FigureManager inst("default");
    return inst;
}

/**
 * Returns a named instance of FigureManager (creates if not exists).
 * Thread-safe via mutex.
 */
FigureManager& FigureManager::instance(const std::string& name_ws) {
    std::string name = trim(name_ws);
    assert(!name.empty());
    if (name == "default") {
        return defaultInstance();
    }

    static std::mutex reg_mtx;
    static std::map<std::string, std::unique_ptr<FigureManager>> registry;

    std::lock_guard<std::mutex> lock(reg_mtx);
    auto it = registry.find(name);
    if (it == registry.end()) {
        auto* raw = new FigureManager(name);
        registry[name] = std::unique_ptr<FigureManager>(raw);
        return *raw;
    }
    return *(it->second);
}

/**
 * Adds a new figure and returns its assigned ID (1-based).
 */
uint32_t FigureManager::addFigure(const Figure& f) {
    std::lock_guard<std::mutex> lock(_mtx);
    _figures.push_back(f);
    return ++_counter;
}

/**
 * Get figure by index.
 */

const Figure& FigureManager::getFigure(size_t i) const {
    return _figures.at(i);
}

/**
 * Returns the number of figures currently stored.
 */
size_t FigureManager::count() const {
    return _figures.size();
}

/**
 * Returns the current counter (last assigned ID).
 */
uint32_t FigureManager::counter() const {
    return _counter;
}

// ---------------- Template Rendering Helpers ----------------


/**
 * Renders the index page listing all figures using 'index.mustache.html'.
 */
static std::string createHtmlIndex(const FigureManager& mgr) {
    mustache::data ctx;
    const size_t n = mgr.count();
    ctx.set("count", std::to_string(n));
    ctx.set("has_figures", n > 0 ? "true" : ""); // for conditional block

    mustache::data figures = mustache::list();
    for (size_t i = 0; i < n; ++i) {
        const auto& f = mgr.getFigure(i);
        mustache::data row;
        row.set("index1", std::to_string(i + 1)); // 1-based index
        row.set("title", f.title());
        figures.push_back(row);
    }
    ctx.set("figures", figures);

    mustache::mustache tmpl(templates::index_mustache_html);

    if (!tmpl.is_valid()) {
        return std::string("Template error: ") + tmpl.error_message();
    }

    return tmpl.render(ctx);
}


// ---------------- HTTP Server (Only on Linux) ----------------

/**
 * Starts an embedded HTTP server to serve figure pages.
 * Routes:
 *   GET /           -> index of all figures
 *   GET /figure/{n} -> individual figure page
 *   GET /health     -> health check (JSON)
 */
void FigureManager::serve(int port) {
#ifdef __linux__

    httplib::Server svr;

    // Route: Home page (list all figures)
    svr.Get("/", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(_mtx);
        res.set_content(createHtmlIndex(*this), "text/html; charset=utf-8");
    });

    // Route: Health check
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"ok\":true}", "application/json");
    });

    // Route: View individual figure by 1-based index
    svr.Get(R"(/figure/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        size_t idx = 0;
        try {
            idx = static_cast<size_t>(std::stoul(req.matches[1].str()));
        } catch (...) {
            res.status = 400;
            res.set_content("Invalid figure index", "text/plain");
            return;
        }

        std::lock_guard<std::mutex> lock(_mtx);
        if (idx == 0 || idx > _figures.size()) {
            res.status = 404;
            res.set_content("Figure not found", "text/plain");
            return;
        }

        const Figure& fig = _figures[idx - 1];
        std::string html = fig.render();
        res.set_content(html, "text/html; charset=utf-8");
    });

    // Enable CORS for external clients
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });

    std::cout << fmt::format("[serve] Starting server on http://127.0.0.1:{}/\n", port);
    std::cout << fmt::format("[routes] /, /figure/{{id}}, /health\n");

    svr.listen("0.0.0.0", port);

#else
    std::cerr << "[serve] HTTP server is only supported on Linux.\n";
#endif
}

// ---------------- Static Export ----------------

/**
 * Saves a static HTML index page to the given file path.
 * Useful for offline sharing.
 */
void FigureManager::save(const std::string& path) {
    std::lock_guard<std::mutex> lock(_mtx);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        std::cerr << "[save] Cannot open file: " << path << "\n";
        return;
    }

    std::string html = createHtmlIndex(*this);
    ofs << html;
    ofs.close();

    std::cout << "[save] Static index page written to: " << path << "\n";
}

} // namespace gsp