#ifndef LIBGSP_UTILS_BASE64_H
#define LIBGSP_UTILS_BASE64_H
#pragma once

#include <string>
#include <vector>

namespace gsp::utils {
    void base64_encode(std::string & out, const std::vector<uint8_t>& buf);
    void base64_encode(std::string & out, const uint8_t* buf, size_t bufLen);
    void base64_encode(std::string & out, std::string const& buf);

    void base64_decode(std::vector<uint8_t> & out, std::string const& encoded_string);
    void base64_decode(std::string & out, std::string const& encoded_string);
} // namespace gsp::utils


#endif // LIBGSP_UTILS_BASE64_H
