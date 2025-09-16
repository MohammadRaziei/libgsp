//
// Created by mohammad on 9/2/25.
//

#include "libgsp/io/File.h"
#include "libgsp/utils/Logging.h"

#include <filesystem>


namespace fs = std::filesystem;

gsp::logging::Logger _logger = gsp::logging::getLoggerByPath(__FILE__);

std::string gsp::io::readFile(const std::string& filename) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        _logger->error("Unable to open file: {}", filename);
        return "";
    }

    // Move the file pointer to the end to get the size of the file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Create a vector to hold the content of the file
    std::vector<char> content(fileSize);

    // Read the file content into the vector
    file.read(content.data(), fileSize);

    // Convert the vector into a string and return it
    return std::string(content.begin(), content.end());
}


void gsp::io::writeFile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        _logger->error("Unable to open file for writing: {}", filename);
        return;
    }

    file.write(data.c_str(), data.size());
}

/**
 * Loads a Mustache template file from the 'templates/' directory.
 * Returns error string if file is missing.
 */
/*
std::string gsp::io::loadTemplate(const std::string& filename) {
    const auto template_path = fs::path(__FILE__).parent_path() / "templates" / filename;
    if (!fs::exists(template_path)) {
        return fmt::format("<h3>Template not found: {}</h3>", template_path.string());
    }
    return gsp::io::readFile(template_path.string());
}
*/