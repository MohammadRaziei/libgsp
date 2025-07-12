#include <sstream>
#include <string>
#include <stdexcept>

// Generic conversion
template <typename T>
T string_to(const std::string& str) {
    std::istringstream ss(str);
    T value;
    if (!(ss >> value))
        throw std::runtime_error("Conversion failed: " + str);
    return value;
}

// Specialization for std::string (no conversion needed)
template <>
std::string string_to<std::string>(const std::string& str) {
    return str;
}

// Optional: Specialization for bool
template <>
bool string_to<bool>(const std::string& str) {
    return (str == "1" || str == "true" || str == "True");
}
