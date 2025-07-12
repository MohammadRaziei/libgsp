
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

// Function to read a file into a string
std::string readFile(const std::string& filename) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Unable to open file: " << filename << std::endl;
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

void writeFile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
        return;
    }

    file.write(data.c_str(), data.size());
}