//
// Created by Mohammad on 7/24/2025.
//

#ifndef LIBGSP_TYPES_H
#define LIBGSP_TYPES_H
#pragma once

#include <type_traits>

#include <ap.h>


namespace gsp::types {
template <typename Matrix> struct typeofelement { using type = void; };
template <> struct typeofelement<alglib::integer_1d_array> { using type = alglib::ae_int_t; };
template <> struct typeofelement<alglib::integer_2d_array> { using type = alglib::ae_int_t; };
template <> struct typeofelement<alglib::complex_1d_array> { using type = alglib::complex; };
template <> struct typeofelement<alglib::complex_2d_array> { using type = alglib::complex; };
template <> struct typeofelement<alglib::real_1d_array> { using type = double; };
template <> struct typeofelement<alglib::real_2d_array> { using type = double; };
template <> struct typeofelement<alglib::sparsematrix> { using type = double; };
template <typename Matrix> using elem_t = typename gsp::types::typeofelement<Matrix>::type;

template <typename T> struct is_alglibmatrix : std::false_type {};
template <> struct is_alglibmatrix<alglib::integer_2d_array> : std::true_type {};
template <> struct is_alglibmatrix<alglib::complex_2d_array> : std::true_type {};
template <> struct is_alglibmatrix<alglib::real_2d_array> : std::true_type {};
template <> struct is_alglibmatrix<alglib::sparsematrix> : std::true_type {};

template <typename T> struct is_alglibvector : std::false_type {};
template <> struct is_alglibvector<alglib::integer_1d_array> : std::true_type {};
template <> struct is_alglibvector<alglib::complex_1d_array> : std::true_type {};
template <> struct is_alglibvector<alglib::real_1d_array> : std::true_type {};
template <> struct is_alglibvector<alglib::sparsematrix> : std::true_type {};
}

#endif  // LIBGSP_TYPES_H
