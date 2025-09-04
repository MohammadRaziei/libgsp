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
#include <functional>

#include "mustache.hpp"           // for Mustache templating
#include "fmt/fmt.h"              // for string formatting

#include "libgsp/utils/string.h"  // for trim()

#include "templates/index_mustache_html.h"
#include "templates/libgsp_logo_svg.h"

namespace fs = std::filesystem;
using namespace kainjow;

// ---------------- Template Rendering Helpers ----------------

/**
 * Renders the index page listing all figures using 'index.mustache.html'.
 */
std::string createHtmlIndex(const gsp::FigureManager& mgr) {
    mustache::data ctx;

    const size_t n = mgr.count();
    ctx.set("count", std::to_string(n));
    ctx.set("has_figures", n > 0 ? "true" : "");

    const std::string& name = mgr.name().empty() ? std::string("default") : mgr.name();
    ctx.set("figuremanager_name", name);

    ctx.set("logo_svg", templates::libgsp_logo_svg);

    mustache::data figures = mustache::list();
    for (size_t i = 0; i < n; ++i) {
        const auto& f = mgr.getFigure(i);
        mustache::data row;
        row.set("index1", std::to_string(i + 1));
        row.set("title",  f.title());
        figures.push_back(row);
    }
    ctx.set("figures", figures);

    mustache::mustache tmpl(templates::index_mustache_html);
    if (!tmpl.is_valid()) {
        return std::string("Template error: ") + tmpl.error_message();
    }
    return tmpl.render(ctx);
}


// ---------------- FileManagerRoute Utility ----------------
#ifdef __linux__
#include "httplib.h"              // cpp-httplib (header-only)

class FileManagerRoute {
   public:
    using LockFn = std::function<std::unique_lock<std::mutex>()>;

    FileManagerRoute(gsp::FigureManager* mgr, LockFn lock_fn)
        : _mgr(mgr), _lock_fn(std::move(lock_fn)) {}

    void registerRoutes(httplib::Server& svr) const {
        // Index
        svr.Get("/", [this](const httplib::Request&, httplib::Response& res) {
            auto guard = _lock_fn();
            res.set_content(createHtmlIndex(*_mgr), "text/html; charset=utf-8");
        });

        // Status (health + meta info)
        svr.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
            std::string figures_json;
            figures_json.reserve(_mgr->count() * 15);

            for (size_t i = 0; i < _mgr->count(); ++i) {
                if (i > 0) figures_json += ",";
                figures_json += fmt::format("\"/figure/{}\"", i + 1);
            }

            auto json = fmt::format(
                "{{\"ok\":true, \"instance\":\"{}\", \"count\":{}, \"figures\":[{}]}}",
                _mgr->name(), _mgr->count(), figures_json
            );

            res.set_content(json, "application/json");
        });

        // Individual figure
        svr.Get(R"(/figure/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            size_t idx = 0;
            try {
                idx = static_cast<size_t>(std::stoul(req.matches[1].str()));
            } catch (...) {
                res.status = 400;
                res.set_content("Invalid figure index", "text/plain");
                return;
            }

            auto guard = _lock_fn();
            if (idx == 0 || idx > _mgr->count()) {
                res.status = 404;
                res.set_content("Figure not found", "text/plain");
                return;
            }

            const auto& fig = _mgr->getFigure(idx - 1);
            std::string html = fig.render();
            res.set_content(html, "text/html; charset=utf-8");
        });
    }

   private:
    gsp::FigureManager* _mgr;
    LockFn _lock_fn;
};

#endif


namespace gsp {

// ---------------- FigureManager Implementation ----------------

FigureManager::FigureManager(const std::string& name) : _name(name) {
    _figures.reserve(4);
}

FigureManager& FigureManager::defaultInstance() {
    static FigureManager inst("default");
    return inst;
}

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

size_t FigureManager::addFigure(const Figure& f) {
    std::lock_guard<std::mutex> lock(_mtx);
    _figures.push_back(f);
    return _figures.size();
}

const Figure& FigureManager::getFigure(size_t i) const {
    return _figures.at(i);
}

size_t FigureManager::count() const {
    return _figures.size();
}

const std::string& FigureManager::name() const {
    return _name;
}

// ---------------- HTTP Server (Only on Linux) ----------------

void FigureManager::serve(int port) {
#ifdef __linux__
    httplib::Server svr;

    FileManagerRoute route(
        this,
        [this]() { return std::unique_lock<std::mutex>(_mtx); }
    );
    route.registerRoutes(svr);

    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });

    std::cout << fmt::format("[serve] Starting server on http://127.0.0.1:{}/\n", port);
    std::cout << "[routes] /, /figure/{id}, /status\n";

    svr.listen("0.0.0.0", port);
#else
    std::cerr << "[serve] HTTP server is only supported on Linux.\n";
#endif
}

// ---------------- Static Export ----------------

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
