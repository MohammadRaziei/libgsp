//
// Created by Mohammad on 7/24/2025.
//

#include "libgsp/utils/matrix.h"

#include <linalg.h>


template <>
void gsp::matrix::allocate(alglib::sparsematrix& matrix, uint32_t rows, uint32_t cols) {
    alglib::sparsecreate(rows, cols, matrix);
}
template <typename Matrix>
void gsp::matrix::allocate(Matrix& matrix, uint32_t rows, uint32_t cols) {
    matrix.setlength(rows, cols);
}
template void gsp::matrix::allocate(alglib::real_2d_array&, uint32_t, uint32_t);
template void gsp::matrix::allocate(alglib::complex_2d_array&, uint32_t, uint32_t);


template <>
void gsp::matrix::setElement(alglib::sparsematrix& matrix, uint32_t row, uint32_t col, double el){
    alglib::sparseset(matrix, row, col, el);
}
template<typename Matrix>
void gsp::matrix::setElement(Matrix& matrix, uint32_t row, uint32_t col, gsp::types::elem_t<Matrix> el) {
    matrix(row, col) = el;
}
template void gsp::matrix::setElement(alglib::real_2d_array&, uint32_t, uint32_t, double);
template void gsp::matrix::setElement(alglib::complex_2d_array&, uint32_t, uint32_t, alglib::complex);


template <>
double gsp::matrix::getElement(alglib::sparsematrix& matrix, uint32_t row, uint32_t col){
    return alglib::sparseget(matrix, row, col);
}
template<typename Matrix>
typename gsp::types::elem_t<Matrix> gsp::matrix::getElement(Matrix& matrix, uint32_t row, uint32_t col) {
    return matrix(row, col);
}
template double gsp::matrix::getElement(alglib::real_2d_array&, uint32_t, uint32_t);
template alglib::complex gsp::matrix::getElement(alglib::complex_2d_array&, uint32_t, uint32_t);

