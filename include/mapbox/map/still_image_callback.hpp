#pragma once

#include <functional>
#include <mapbox/common/optional.hpp>
#include <string>

namespace mapbox {
namespace map {

using StillImageCallback = std::function<void(const mapbox::common::optional<std::string>&)>;

} // namespace map
} // namespace mapbox
