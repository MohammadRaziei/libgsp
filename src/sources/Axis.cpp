//
// Created by Mohammad on 8/24/2025.
//

#include "libgsp/plotting/Axis.h"

#include <fmt/fmt.h>
#include <mustache.hpp>
#include <string>

namespace gsp {

// ------------------------
// Implementation (Pimpl)
// ------------------------
struct Axis::Impl {
    std::string title;
    bool has_title = false;
    std::string width = "100%";
    std::string height = "400px";
    std::string body_html;

    // Cache
    mutable bool _dirty = true;
    mutable bool _dirty_head = true;
    mutable bool _dirty_body = true;
    mutable std::string _rendered;
    mutable std::string _rendered_head;
    mutable std::string _rendered_body;

    // --- Templates ---
    static const std::string& head_template() {
        static const std::string tpl = R"(
<div class="axis-header" style="font-weight:700;padding:10px 12px;border-bottom:1px solid #f0f0f0;background:#fafafa;color:#333;">
  {{title}}
</div>
)";
        return tpl;
    }

    static const std::string& body_template() {
        static const std::string tpl = R"(
<div class="axis-body" style="padding:8px;width:{{width}};height:{{height}};min-height:200px;">
  {{{content}}}
</div>
)";
        return tpl;
    }

    // --- Render Head ---
    std::string renderHead() const {
        if (!_dirty_head && !_rendered_head.empty()) {
            return _rendered_head;
        }

        if (!has_title) {
            _rendered_head.clear();
            return _rendered_head;
        }

        try {
            kainjow::mustache::mustache tpl(head_template());
            kainjow::mustache::data data;
            data["title"] = title;
            _rendered_head = tpl.render(data);
            _dirty_head = false;
        } catch (...) {
            _rendered_head = "<div class=\"axis-header\">[Title Error]</div>";
            _dirty_head = false;
        }
        return _rendered_head;
    }

    // --- Render Body ---
    std::string renderBody() const {
        if (!_dirty_body && !_rendered_body.empty()) {
            return _rendered_body;
        }

        try {
            kainjow::mustache::mustache tpl(body_template());
            kainjow::mustache::data data;
            data["width"] = width;
            data["height"] = height;
            data["content"] = body_html;

            _rendered_body = tpl.render(data);
            _dirty_body = false;
        } catch (...) {
            _rendered_body = fmt::format(
                R"(<div class="axis-body" style="width:{};height:{};background:#f9f9f9;color:#999;text-align:center;padding:20px;">Render failed</div>)",
                width, height
            );
            _dirty_body = false;
        }
        return _rendered_body;
    }

    // --- Full Render ---
    std::string render() const {
        if (!_dirty && !_rendered.empty()) {
            return _rendered;
        }

        std::string head = renderHead();
        std::string body = renderBody();

        _rendered.clear();
        if (!head.empty()) _rendered += head;
        if (!body.empty()) _rendered += body;

        _dirty = false;
        return _rendered;
    }

    // --- Mark dirty on change ---
    void markDirty() {
        _dirty = true;
        _dirty_head = true;
        _dirty_body = true;
    }
};

// ------------------------
// Public Methods
// ------------------------

Axis::Axis(const std::string& width, const std::string& height)
    : _impl(std::make_shared<Impl>()) {
    _impl->width = width;
    _impl->height = height;
}

Axis::~Axis() = default;
Axis::Axis(const Axis&) = default;
Axis& Axis::operator=(const Axis&) = default;

Axis& Axis::setTitle(const std::string& title) {
    _impl->title = title;
    _impl->has_title = true;
    _impl->markDirty();
    return *this;
}

bool Axis::hasTitle() const {
    return _impl->has_title;
}

const std::string& Axis::title() const {
    return _impl->title;
}

Axis& Axis::setWidth(const std::string& width) {
    if (width.empty()) return *this;
    _impl->width = width;
    _impl->markDirty();
    return *this;
}

Axis& Axis::setHeight(const std::string& height) {
    if (height.empty()) return *this;
    _impl->height = height;
    _impl->markDirty();
    return *this;
}

Axis& Axis::setSize(const std::string& width, const std::string& height) {
    setWidth(width);
    setHeight(height);
    return *this;
}

const std::string& Axis::width() const {
    return _impl->width;
}

const std::string& Axis::height() const {
    return _impl->height;
}

Axis& Axis::setBodyHtml(const std::string& html) {
    _impl->body_html = html;
    _impl->markDirty();
    return *this;
}

const std::string& Axis::bodyHtml() const {
    return _impl->body_html;
}

std::string Axis::render() const {
    return _impl->render();
}

std::string Axis::renderHead() const {
    return _impl->renderHead();
}

std::string Axis::renderBody() const {
    return _impl->renderBody();
}

} // namespace gsp