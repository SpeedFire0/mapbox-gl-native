#pragma once

#include <mapbox/common/config.hpp>

namespace mapbox {
namespace map {

enum class MapChange {
    DidFinishLoadingMap,
    DidFinishLoadingStyle
};

} // namespace map
} // namespace mapbox
