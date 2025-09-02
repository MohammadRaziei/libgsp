#ifndef LIBGSP_FILE_H
#define LIBGSP_FILE_H
#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

namespace gsp::io {

// Function to read a file into a string
std::string readFile(const std::string& filename);
std::string loadTemplate(const std::string& filename);

void writeFile(const std::string& filename, const std::string& data);

} // namespace gsp::io
#endif // LIBGSP_FILE_H
