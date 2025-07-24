//
// Created by Mohammad on 7/24/2025.
//

#ifndef LIBGSP_MATRIX_H
#define LIBGSP_MATRIX_H
#pragma once

#include <cstdint>

#include "utils/types.h"


namespace gsp::matrix {

#include <type_traits>


template<typename Matrix> void allocate(Matrix& matrix, uint32_t rows, uint32_t cols);

template<typename Matrix, typename Elem = typename gsp::types::typeofelement<Matrix>::type>
void setElement(Matrix& matrix, uint32_t row, uint32_t col, Elem el);

template<typename Matrix, typename Elem = typename gsp::types::typeofelement<Matrix>::type>
Elem getElement(Matrix& matrix, uint32_t row, uint32_t col);
}


#endif  // LIBGSP_MATRIX_H


