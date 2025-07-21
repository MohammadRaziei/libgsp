#include <sstream>
#include <string>
#include <stdexcept>
#include <map>

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


template <typename T>
T map_get(const std::map<std::string, std::string>& m, const std::string& key, const T& default_value) {
    auto it = m.find(key);
    if (it != m.end()) {
        return string_to<T>(it->second);
    }
    return default_value;
}