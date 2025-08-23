//
// Created by Mohammad on 8/23/2025.
//

#ifndef LIBGSP_STRING_H
#define LIBGSP_STRING_H
#pragma once

#include <string>


std::string trim(const std::string& s) {
    // Find the first non-whitespace character from the beginning
    std::string whitespace = " \t\n\r\f\v";
    size_t first = s.find_first_not_of(whitespace);

    // If no non-whitespace character is found, the string is all whitespace
    if (std::string::npos == first) {
        return s; // Or return an empty string if desired for all-whitespace input
    }

    // Find the last non-whitespace character from the end
    size_t last = s.find_last_not_of(whitespace);

    // Extract the substring between the first and last non-whitespace characters
    return s.substr(first, (last - first + 1));
}

#endif  // LIBGSP_STRING_H
