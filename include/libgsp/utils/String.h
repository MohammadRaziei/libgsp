//
// Created by Mohammad on 8/23/2025.
//

#ifndef LIBGSP_STRING_H
#define LIBGSP_STRING_H
#pragma once

#include <string>

namespace gsp::utils {
// Trim leading and trailing whitespace from a string
std::string trim(const std::string& s);
// Compress HTML by removing unnecessary whitespace and optionally comments
// - Preserves single spaces in text content (outside tags)
// - Preserves single spaces in attributes (inside tags with quotes)
// - Removes all other whitespace
// - If removeComments is true, removes HTML comments <!-- ... -->
std::string compressHTML(const std::string& html, bool removeComments = false);
}  // namespace gsp::utils

#endif  // LIBGSP_STRING_H
