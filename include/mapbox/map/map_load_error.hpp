#pragma once

#include <mapbox/common/config.hpp>

namespace mapbox {
namespace map {

enum class MapLoadError {
    StyleParseError,
    StyleLoadError,
    NotFoundError,
    UnknownError
};

} // namespace map
} // namespace mapbox
