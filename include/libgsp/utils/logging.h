//
// Created by mohammad on 9/9/25.
//

#ifndef LIBGSP_LOGGING_H
#define LIBGSP_LOGGING_H
#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace gsp::logging {
// Configure the global/default logger once in main()
void setDefaultConfigs(spdlog::level::level_enum level = spdlog::level::info,
                       const std::string& pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

std::shared_ptr<spdlog::logger> getLogger(const std::string& name = "root");
} // namespace gsp::logging

#endif  // LIBGSP_LOGGING_H
