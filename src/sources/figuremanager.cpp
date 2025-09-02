//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"

#include <iostream>
#include <map>
#include <memory>
#include <filesystem>
#include <sstream>      // for ostrings
#include <thread>       // optional if you later want async
#include <fstream>  // for std::ofstream
#include <mustache.hpp>

#include "libgsp/plotting/figuremanager.h"

using namespace kainjow;

#ifdef __linux__
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <httplib.h>    // <--- cpp-httplib (header-only)
#endif

#include <fmt/fmt.h>

#include "libgsp/utils/string.h"
#include "libgsp/io/file.h"

namespace fs = std::filesystem;


// ---------------- FigureManager ----------------
gsp::FigureManager::FigureManager(const std::string& name) : _counter(0), _name(name) {}

gsp::FigureManager& gsp::FigureManager::defaultInstance() {
    static FigureManager inst("default");
    return inst;
}

gsp::FigureManager& gsp::FigureManager::instance(const std::string& name_ws) {
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
        auto* raw = new FigureManager(name);     // allocate
        registry[name] = std::unique_ptr<FigureManager>(raw);  // wrap in unique_ptr
        return *raw;
    }
    return *(it->second);
}

uint32_t gsp::FigureManager::addFigure(const gsp::Figure& f) {
    std::lock_guard<std::mutex> lock(_mtx);
    _figures.push_back(f);
    return ++_counter;
}

const gsp::Figure& gsp::FigureManager::getFigure(size_t i) const { return _figures.at(i); }
gsp::Figure& gsp::FigureManager::getFigure(size_t i) { return _figures.at(i); }

size_t gsp::FigureManager::count() const { return _figures.size(); }
uint32_t gsp::FigureManager::counter() const { return _counter; }

// --- tiny helpers to build html pages ---

// ---------------- layout ----------------
static std::string html_layout(const std::string& title, const std::string& body) {
    return fmt::format(
        R"(<!doctype html>
<html lang="en"><head>
<meta charset="utf-8" />
<title>{}</title>
<meta name="viewport" content="width=device-width,initial-scale=1" />
<style>
  body{{font-family:system-ui,Segoe UI,Roboto,Helvetica,Arial,sans-serif;margin:24px}}
  a{{color:#06f;text-decoration:none}}
  a:hover{{text-decoration:underline}}
  .grid{{display:grid;gap:8px}}
  .card{{padding:12px 16px;border:1px solid #e5e7eb;border-radius:10px}}
  code{{background:#f4f4f5;padding:2px 6px;border-radius:6px}}
</style>
</head><body>
{}
</body></html>)",
        title, body
    );
}

// ---------------- index page ----------------
static std::string html_index(const gsp::FigureManager& mgr) {
    std::string body;
    body += "<h1>Figure Manager</h1>\n";
    body += fmt::format("<p>Total figures: <b>{}</b></p>\n", mgr.count());
    body += "<div class=\"grid\">\n";

    for (size_t i = 0; i < mgr.count(); ++i) {
        const auto& f = mgr.getFigure(i);
        body += fmt::format(
            R"(<div class="card">
  <div><b>#{}</b> &mdash; {}</div>
  <div><a href="/figure/{}">/figure/{}</a></div>
</div>
)",
            i + 1, f.title(), i + 1, i + 1
        );
    }

    if (mgr.count() == 0) {
        body += "<div class=\"card\">No figures yet. POST or add via API.</div>\n";
    }

    body += "</div>\n<p><small>Health: <code>/health</code></small></p>";

    return html_layout("Figure Manager", body);
}

// ---------------- single figure page ----------------
static std::string html_figure(const gsp::Figure& fig, size_t idx) {
    std::string body = fmt::format(
        R"(<h1>Figure #{}</h1>
<p>Title: <b>{}</b></p>
<p><a href="/">&larr; back</a></p>
<div class="card">
  This is a placeholder page for this figure.
  You can embed your Three.js HTML here via a template.
</div>
)",
        idx, fig.title()
    );

    return html_layout(fmt::format("Figure {}", idx), body);
}




static std::string createHtmlIndex(const gsp::FigureManager& mgr) {
    // 1) build mustache data
    mustache::data ctx;
    const size_t n = mgr.count();
    ctx.set("count", std::to_string(n));
    ctx.set("has_figures", n > 0 ? "true" : "");

    mustache::data figures = mustache::list();
    for (size_t i = 0; i < n; ++i) {
        const auto& f = mgr.getFigure(i);
        mustache::data row;
        row.set("index1", std::to_string(i + 1)); // 1-based index
        row.set("title",  f.title());             // will be HTML-escaped by default
        figures.push_back(row);
    }
    ctx.set("figures", figures);

    // 2) render template
    const auto template_path = fs::path(__FILE__).parent_path()/ "templates" / "index.mustache.html";
    const std::string tmpl_text = gsp::io::readFile(template_path.string());  // adjust path if needed
    mustache::mustache tmpl(tmpl_text);

    // optional: check validity
    if (!tmpl.is_valid()) {
        return std::string("Template error: ") + tmpl.error_message();
    }

    return tmpl.render(ctx);
}



void gsp::FigureManager::serve(int port) {
#ifdef __linux__

    httplib::Server svr;

    // GET / -> index listing
    svr.Get("/", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(_mtx);
        res.set_content(createHtmlIndex(*this), "text/html; charset=utf-8");
    });

    // GET /health -> simple health probe
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"ok\":true}", "application/json");
    });

    // GET /figure/{i} (1-based index)
    svr.Get(R"(/figure/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        size_t idx = 0;
        try {
            idx = static_cast<size_t>(std::stoul(req.matches[1].str())); // 1..N
        } catch (...) {
            res.status = 400;
            res.set_content("Bad index", "text/plain");
            return;
        }
        std::lock_guard<std::mutex> lock(_mtx);
        if (idx == 0 || idx > _figures.size()) {
            res.status = 404;
            res.set_content("Figure not found", "text/plain");
            return;
        }
        const Figure& fig = _figures[idx - 1];
        res.set_content(html_figure(fig, idx), "text/html; charset=utf-8");
    });

    // CORS (optional)
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });

    std::cout << "[serve] http://127.0.0.1:" << port << "  (routes: /, /figure/{i}, /health)\n";
    // blocking; if you want async, run in a std::thread
    svr.listen("0.0.0.0", port);

#endif
}

void gsp::FigureManager::save(const std::string& path) {
    // You can dump a static index page with links to per-figure HTML
    std::lock_guard<std::mutex> lock(_mtx);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        std::cerr << "[save] cannot open: " << path << "\n";
        return;
    }
    ofs << html_index(*this);
    std::cout << "[save] wrote index to " << path << "\n";
}








