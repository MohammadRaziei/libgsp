//
// Created by mohammad on 9/9/25.
//

#ifndef LIBGSP_LOGGING_H
#define LIBGSP_LOGGING_H
#pragma once

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wextra"
#elif defined(_MSC_VER)
#  pragma warning(push, 0)
#endif

#include <spdlog/spdlog.h>
#include <fmt/ostr.h>

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#include <memory>
#include <string>

#if !defined(__PRETTY_FUNCTION__) && defined(__FUNCSIG__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

inline std::string _methodName(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin,end) + "()";
}

#define __METHOD_NAME__ _methodName(__PRETTY_FUNCTION__)


namespace gsp::logging {
using level = spdlog::level::level_enum;
using Logger = std::shared_ptr<spdlog::logger>;
// Configure the global/default logger once in main()
void basicConfig(gsp::logging::level level = gsp::logging::level::info,
                      const std::string& pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

void basicConfig(
    int argc, char** argv,
    gsp::logging::level level = gsp::logging::level::info,
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v"
);

Logger getLogger(const std::string& name = "root");
Logger getLoggerByPath(const std::string& path);
} // namespace gsp::logging

#endif  // LIBGSP_LOGGING_H
