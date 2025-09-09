//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"

#include <iostream>
#include <map>
#include <memory>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <mutex>
#include <functional>

#include "mustache.hpp"           // for Mustache templating
#include "fmt/fmt.h"              // for string formatting

#include "libgsp/utils/string.h"  // for trim()

#include "templates/index_mustache_html.h"
#include "templates/assets/libgsp_logo_svg.h"

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

    ctx.set("logo_svg", templates::assets::libgsp_logo_svg);

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
        const std::string msg = fmt::format("Template error: {}", tmpl.error_message());
        mgr._logger->error(msg);
        return msg;
    }
    return tmpl.render(ctx);
}


// ---------------- FileManagerRoute Utility ----------------
#ifdef __linux__
#include <httplib.h>
#include <string>


#define ROUTE_GET(PATH, METHOD_PTR) _svr.Get((PATH), [this](const httplib::Request& req, httplib::Response& res) { METHOD_PTR(req, res); })

class FileManagerServer {
public:
    FileManagerServer(gsp::FigureManager* mgr, std::mutex& mtx)
        : _mgr(mgr), _mtx(mtx) {
        assert(_mgr != nullptr);

        _svr.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Headers", "Content-Type"},
        });

        ROUTE_GET("/", getIndexRoute);
        ROUTE_GET("/status", getStatusRoute);
        ROUTE_GET(R"(/figure/(\d+))",  getFigureRoute);
    }

    bool listen(const char* host, const int port) { return _svr.listen(host, port); }
    httplib::Server& server() { return _svr; }

private:
    // no Request needed
    void getIndexRoute(const httplib::Request&, httplib::Response& res) const {
        std::lock_guard<std::mutex> g(_mtx);

        _mgr->_logger->debug("GET Index");

        res.set_content(createHtmlIndex(*_mgr), "text/html; charset=utf-8");
    }

    // no Request needed
    void getStatusRoute(const httplib::Request&, httplib::Response& res) const {
        std::lock_guard<std::mutex> g(_mtx);

        _mgr->_logger->debug("GET Status");

        const auto c = _mgr->count();

        std::string figs;
        figs.reserve(c * 16);
        for (size_t i = 0; i < c; ++i) {
            if (i) figs += ',';
            figs += fmt::format("\"/figure/{}\"", i + 1);
        }

        const std::string inst = _mgr->name().empty() ? "default" : _mgr->name();
        auto json = fmt::format(
            "{{\"ok\":true,\"instance\":\"{}\",\"count\":{},\"figures\":[{}]}}",
            inst, c, figs
        );
        res.set_content(json, "application/json");
    }

    // needs Request (path param)
    void getFigureRoute(const httplib::Request& req, httplib::Response& res) const {
        size_t idx1{};
        try {
            idx1 = static_cast<size_t>(std::stoul(req.matches[1].str()));
        } catch (...) {
            res.status = 400;
            _mgr->_logger->error("Invalid figure index");
            res.set_content("Invalid figure index", "text/plain; charset=utf-8");
            return;
        }

        _mgr->_logger->debug("GET Figure {}", idx1);


        std::lock_guard<std::mutex> g(_mtx);
        if (idx1 == 0 || idx1 > _mgr->count()) {
            res.status = 404;
            _mgr->_logger->error("Figure not found");
            res.set_content("Figure not found", "text/plain; charset=utf-8");
            return;
        }

        const auto& fig = _mgr->getFigure(idx1 - 1);
        res.set_content(fig.render(), "text/html; charset=utf-8");
    }

private:
    gsp::FigureManager* _mgr;
    std::mutex& _mtx;
    httplib::Server _svr;
};
#endif


namespace gsp {

// ---------------- FigureManager Implementation ----------------

FigureManager::FigureManager(const std::string& name) : _name(name),
        _logger(gsp::logging::getLogger("FigureManager")){
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
    FileManagerServer server(this, _mtx);

    _logger->info("Starting server on http://127.0.0.1:{}/", port);
    _logger->info("routes: /, /figure/{id}, /status");
    server.listen("0.0.0.0", port);

#else
    _logger->warn("HTTP server is only supported on Linux.");
#endif
}

// ---------------- Static Export ----------------

void FigureManager::save(const std::string& path) {
    std::lock_guard<std::mutex> lock(_mtx);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        _logger->error("Cannot open file: {}", path);
        return;
    }

    std::string html = createHtmlIndex(*this);
    ofs << html;
    ofs.close();

    _logger->info("Static index page written to: {}", path);
}

} // namespace gsp
