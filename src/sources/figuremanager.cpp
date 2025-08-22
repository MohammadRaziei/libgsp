//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/plotting/figuremanager.h"

#include <iostream>
#include <map>
#include <memory>

// ---------------- Figure ----------------
Figure::Figure(const std::string& title) : _title(title) {}
const std::string& Figure::title() const { return _title; }

// ---------------- FigureManager ----------------
FigureManager::FigureManager() : _counter(0) {}

FigureManager& FigureManager::defaultInstance() {
    static FigureManager inst;
    return inst;
}

FigureManager& FigureManager::instance(const std::string& name) {
    if (name.empty()) {
        return defaultInstance();
    }
    static std::mutex reg_mtx;
    static std::map<std::string, std::unique_ptr<FigureManager>> registry;

    std::lock_guard<std::mutex> lock(reg_mtx);
    auto it = registry.find(name);
    if (it == registry.end()) {
        auto* raw = new FigureManager();     // allocate
        registry[name] = std::unique_ptr<FigureManager>(raw);  // wrap in unique_ptr
        return *raw;
    }
    return *(it->second);
}

uint32_t FigureManager::addFigure(const Figure& f) {
    std::lock_guard<std::mutex> lock(_mtx);
    _figures.push_back(f);
    return ++_counter;
}

const Figure& FigureManager::getFigure(size_t i) const { return _figures.at(i); }
Figure& FigureManager::getFigure(size_t i) { return _figures.at(i); }

size_t FigureManager::count() const { return _figures.size(); }
uint32_t FigureManager::counter() const { return _counter; }

void FigureManager::serve(int port) {
    std::cout << "[serve] HTTP server would run on port " << port << "\n";
    for (size_t i=0; i<_figures.size(); ++i) {
        std::cout << "  /figure" << (i+1) << " -> " << _figures[i].title() << "\n";
    }
}

void FigureManager::save(const std::string& path) {
    std::cout << "[save] Writing " << _figures.size()
              << " figures to " << path << "\n";
}
