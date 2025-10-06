//
// Created by mohammad on 9/2/25.
//

#include "libgsp/utils/String.h"


std::string gsp::utils::compressHTML(const std::string& html, bool removeComments) {
    std::string result;
    bool in_tag = false;
    bool in_quote = false;
    bool in_comment = false;
    char quote_char = 0;
    bool last_was_space = false;

    for (size_t i = 0; i < html.length(); i++) {
        char c = html[i];

        // Check for comment start
        if (removeComments && !in_quote && !in_comment &&
            i + 3 < html.length() && c == '<' && html[i+1] == '!' &&
            html[i+2] == '-' && html[i+3] == '-') {
            in_comment = true;
            continue;
        }

        // Check for comment end
        if (in_comment && i + 2 < html.length() && c == '-' &&
            html[i+1] == '-' && html[i+2] == '>') {
            in_comment = false;
            i += 2; // Skip the remaining comment characters
            continue;
        }

        // Skip characters inside comments
        if (in_comment) {
            continue;
        }

        // Handle quotes
        if (c == '"' || c == '\'') {
            if (!in_quote) {
                in_quote = true;
                quote_char = c;
            } else if (c == quote_char) {
                in_quote = false;
            }
            result += c;
            last_was_space = false;
            continue;
        }

        // Handle tag boundaries
        if (c == '<') {
            in_tag = true;
            result += c;
            last_was_space = false;
            continue;
        }

        if (c == '>') {
            in_tag = false;
            result += c;
            last_was_space = false;
            continue;
        }

        // Handle whitespace
        if (std::isspace(c)) {
            // Preserve single space in text content (outside tags)
            if (!in_tag && !in_quote && !last_was_space &&
                result.size() > 0 && !std::isspace(result.back())) {
                result += ' ';
                last_was_space = true;
            }
            // Preserve single space in attributes (inside tags with quotes)
            else if (in_tag && in_quote && !last_was_space) {
                result += ' ';
                last_was_space = true;
            }
            // Skip all other whitespace
            continue;
        }

        // Regular character
        result += c;
        last_was_space = false;
    }

    return result;
}

std::string gsp::utils::trim(const std::string& s) {
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