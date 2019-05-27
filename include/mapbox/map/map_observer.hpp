#pragma once

#include "map_change.hpp"
#include "map_load_error.hpp"
#include <mapbox/common/peer.hpp>
#include <string>

namespace mapbox {
namespace map {

class MapObserver {
public:
    virtual void onMapChanged(MapChange change) = 0;
    virtual void onMapLoadError(MapLoadError error, const std::string& message) = 0;

protected:
    virtual ~MapObserver() = default;

public:
    /// @cond For use only by generated bindings.
    mapbox::common::peer peer;
    /// @endcond
};

} // namespace map
} // namespace mapbox
