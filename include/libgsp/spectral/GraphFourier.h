//
// Created by mohammad on 1/11/26.
//

#ifndef LIBGSP_GRAPHFOURIER_H
#define LIBGSP_GRAPHFOURIER_H
#pragma once

#include "libgsp/spectral/SpectralTypes.h"
#include "libgsp/Graph.h"
#include "libgsp/GraphSignal.h"
#include "libgsp/utils/Logging.h"
//
//namespace gsp {
//    template <class Matrix> class GraphFourierBase;
//}
//
//template <class Matrix>
//class gsp::GraphFourierSymBase {
//public:
//    GraphFourierSymBase(gsp::Graph<Matrix>& graph, gsp::ShiftType shift_type = gsp::ShiftType::Laplacian);
//    virtual ~GraphFourierSymBase();
//    virtual void init() = 0;
//
//    template<typename T> gsp::Signal<T> transform(gsp::Signal<T>& signal);
//    [[ nodiscard ]] std::string method() { return method_; }
//
//protected:
//    GraphFourierSymBase(gsp::Graph<Matrix>& graph, gsp::ShiftType shift_type, std::string method);
//
//    std::string method_;
//    gsp::ShiftType shift_type_;
//    gsp::logging::Logger logger_;
//    Matrix* shift_matrix_ = nullptr;
//};
//
//
//template<class Matrix>
//gsp::GraphFourierSymBase<Matrix>::GraphFourierSymBase(gsp::Graph<Matrix> &graph, gsp::ShiftType shift_type) :
//        GraphFourierSymBase(graph, shift_type, "GraphFourierBase") {
//}
//
//template<class Matrix>
//gsp::GraphFourierSymBase<Matrix>::GraphFourierSymBase(gsp::Graph<Matrix> &graph, gsp::ShiftType shift_type, std::string method) :
//    shift_type_(shift_type), method_(method), logger_(gsp::logging::getLogger(method)) {
//    switch (shift_type) {
//        case gsp::ShiftType::Laplacian:
//            shift_matrix_ = &graph.laplacian();
//            break;
//        case gsp::ShiftType::Weights:
//            shift_matrix_ = &graph.weights();
//            break;
//        case gsp::ShiftType::NormalizedLaplacian:
//            shift_matrix_ = &graph.normalizedLaplacian();
//            break;
//        case gsp::ShiftType::NormalizedWeights:
//            shift_matrix_ = &graph.normalizedWeight();
//            break;
//    }
//}
//
//template<class Matrix>
//void gsp::GraphFourierSymBase<Matrix>::init() {
//}


#endif //LIBGSP_GRAPHFOURIER_H
