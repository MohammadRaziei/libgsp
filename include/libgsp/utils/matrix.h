//
// Created by Mohammad on 7/24/2025.
//

#ifndef LIBGSP_MATRIX_H
#define LIBGSP_MATRIX_H
#pragma once

#include <cstdint>
#include <type_traits>

#include "libgsp/utils/types.h"


namespace gsp::matrix {
template<typename Matrix> void allocate(Matrix& matrix, uint32_t rows, uint32_t cols);

template<typename Matrix>
void setElement(Matrix& matrix, uint32_t row, uint32_t col, gsp::types::elem_t<Matrix> el);

template<typename Matrix>
gsp::types::elem_t<Matrix> getElement(Matrix& matrix, uint32_t row, uint32_t col);
}


#endif  // LIBGSP_MATRIX_H


