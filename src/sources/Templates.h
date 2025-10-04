//
// Created by mohammad on 04.10.25.
//

#ifndef LIBGSP_TEMPLATES_H
#define LIBGSP_TEMPLATES_H

#include "templates/assets/libgsp_logo_ascii_txt.h"
#include "libgsp/utils/GspInfo.h"

#include <fmt/fmt.h>

namespace templates {
    inline std::string getInitialComments() {
        const auto& info = gsp::info::GspInfo::instance();
        return fmt::format("{}\n\n\n{}",
            templates::assets::libgsp_logo_ascii_txt,
            info.str(false));
    }
}

#endif //LIBGSP_TEMPLATES_H