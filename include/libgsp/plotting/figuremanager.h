//
// Created by Mohammad on 8/22/2025.
//

#ifndef LIBGSP_FIGUREMANAGER_H
#define LIBGSP_FIGUREMANAGER_H

#include <vector>
#include <string>
#include <cstdint>
#include <mutex>

#include "libgsp/plotting/figure.h"

namespace gsp {
    class FigureManager {
        public:
        FigureManager(const FigureManager&) = delete;
        // --- default instance (empty name) ---
        static FigureManager& defaultInstance();
        
        // --- named instance registry ---
        static FigureManager& instance(const std::string& name = "default");
        
        // --- API ---
        size_t addFigure(const gsp::Figure& f);
        const gsp::Figure& getFigure(size_t i) const;
        
        size_t count() const;

        void serve(int port=8080);
        void save(const std::string& path);

        const std::string& name() const;

        private:
        FigureManager(const std::string& name);
        
        mutable std::mutex _mtx;
    std::vector<gsp::Figure> _figures;
    std::string _name;
};
} // namespace gsp

#endif // LIBGSP_FIGUREMANAGER_H
