//
// Created by Mohammad on 8/24/2025.
//

#ifndef LIBGSP_AXIS_H
#define LIBGSP_AXIS_H
#pragma once
#include <string>
#include <memory>

namespace gsp {

class Axis {
   public:
    // پیش‌فرض: width = "100%", height = "400px"
    explicit Axis(const std::string& width = "100%", const std::string& height = "400px");
    ~Axis();
    Axis(const Axis&);
    Axis& operator=(const Axis&);

    // --- Title ---
    Axis& setTitle(const std::string& title);
    bool hasTitle() const;
    const std::string& title() const;

    // --- Dimensions (as strings) ---
    Axis& setWidth(const std::string& width);
    Axis& setHeight(const std::string& height);
    Axis& setSize(const std::string& width, const std::string& height);
    const std::string& width() const;
    const std::string& height() const;

    // --- Content ---
    Axis& setBodyHtml(const std::string& html);
    const std::string& bodyHtml() const;

    // --- Rendering (cached) ---
    std::string render() const;
    std::string renderHead() const;
    std::string renderBody() const;

   private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};


} // namespace gsp

#endif  // LIBGSP_AXIS_H
