//
// Created by Mohammad on 7/20/2025.
//

#ifndef LIBGSP_GRAPH_H
#define LIBGSP_GRAPH_H

#include <vector>
#include <ap.h>
#include <stdint.h>



class Graph {
   public:
    Graph(const uint32_t);

   public:
    const uint32_t num_nodes;
    alglib::real_2d_array weights;
    std::vector<std::string> names;
    alglib::real_2d_array coords;
    std::vector<alglib::real_1d_array> signals;
};

#endif  // LIBGSP_GRAPH_H
