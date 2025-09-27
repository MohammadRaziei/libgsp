//
// Created by Mohammad on 9/25/2025.
//

#ifndef LIBGSP_INFO_H
#define LIBGSP_INFO_H

#pragma once
#include <string>
#include <map>

#include "libgsp/utils/Logging.h"

namespace gsp::info {

// Classify submodule location
enum class SubmoduleType {
    Source,
    Test,
    Unknown
};

// Lightweight submodule info
struct Submodule {
    std::string name;   // logical name
    std::string path;   // filesystem path
    std::string url;    // repo URL
    SubmoduleType type = SubmoduleType::Unknown;
};

// Alias for dependency map
using Depenencies = std::map<std::string, gsp::info::Submodule>;

class GspInfo {
public:
    // Singleton accessor
    static GspInfo& instance();
    static void summary();

    // ----- Getters -----
    const std::string& name()        const noexcept { return _name; }
    const std::string& authorName()  const noexcept { return _author_name; }
    const std::string& authorEmail() const noexcept { return _author_email; }
    const std::string& siteUrl()     const noexcept { return _site_url; }
    const std::string& version()     const noexcept { return _version; }
    const std::string& language()    const noexcept { return _language; }
    const std::string& os()          const noexcept { return _os; }

    const Depenencies& dependencies() const noexcept { return _deps; }

    // ----- Setters -----
    GspInfo& setName(std::string v)        { _name = std::move(v); return *this; }
    GspInfo& setVersion(std::string v)     { _version = std::move(v); return *this; }
    GspInfo& setAuthorName(std::string v)  { _author_name = std::move(v); return *this; }
    GspInfo& setAuthorEmail(std::string v) { _author_email = std::move(v); return *this; }
    GspInfo& setLanguage(std::string v)    { _language = std::move(v); return *this; }
    GspInfo& setOs(std::string v)          { _os = std::move(v); return *this; }

    // Utility
    static std::string detectOs();
    void print() const;


private:
    GspInfo(); // private constructor

    // Non-copyable / non-movable
    GspInfo(const GspInfo&) = delete;
    GspInfo& operator=(const GspInfo&) = delete;
    GspInfo(GspInfo&&) = delete;
    GspInfo& operator=(GspInfo&&) = delete;

    void parsePyproject();   // fills _name,_version,_author_name,_author_email,_site_url
    void parseGitmodules();  // fills _deps

    // Data
    std::string _name;
    std::string _author_name;
    std::string _author_email;
    std::string _site_url;
    std::string _version;
    std::string _language;
    std::string _os;

    Depenencies _deps;   // <--- Dependencies live here now

    gsp::logging::Logger _logger;
};

} // namespace gsp::info

#endif  // LIBGSP_INFO_H
