#pragma once

#include <mapbox/common/config.hpp>

#include "constrain_mode.hpp"
#include "mbgl/annotation/annotation.hpp"
#include "mbgl/map/bound_options.hpp"
#include "mbgl/map/camera.hpp"
#include "mbgl/map/map_options.hpp"
#include "mbgl/map/mode.hpp"
#include "mbgl/map/projection_mode.hpp"
#include "mbgl/storage/resource_options.hpp"
#include "mbgl/util/geo.hpp"
#include "mbgl/util/size.hpp"
#include "north_orientation.hpp"
#include "still_image_callback.hpp"
#include "viewport_mode.hpp"
#include <cstdint>
#include <mapbox/common/optional.hpp>
#include <mapbox/common/peer.hpp>
#include <memory>
#include <string>
#include <vector>

namespace mapbox {
namespace map {

class MapImpl;

class MapClient;
class MapObserver;

class MAPBOX_MAP_PUBLIC Map {
public:
    Map(std::shared_ptr<MapClient> client, std::shared_ptr<MapObserver> observer, ::mbgl::MapOptions mapOptions, ::mbgl::ResourceOptions resourceOptions);
    ~Map();

    void createRenderer();
    void destroyRenderer();
    void render();
    void setSize(const ::mbgl::Size& size);
    void setStyleURL(const std::string& styleURL);
    void setDefaultFramebufferObject(uint32_t framebufferId);
    void renderStill(StillImageCallback callback);
    void renderStill(const ::mbgl::CameraOptions& camera, const ::mbgl::MapDebugOptions& options, StillImageCallback callback);
    void triggerRepaint();
    void jumpTo(const ::mbgl::CameraOptions& camera);
    ::mbgl::CameraOptions getCameraOptions(const ::mbgl::EdgeInsets& padding);
    void easeTo(const ::mbgl::CameraOptions& camera, const ::mbgl::AnimationOptions& animation);
    void flyTo(const ::mbgl::CameraOptions& camera, const ::mbgl::AnimationOptions& animation);
    void moveBy(const ::mbgl::ScreenCoordinate& point, const mapbox::common::optional<::mbgl::AnimationOptions>& animation);
    void scaleBy(double scale, const mapbox::common::optional<::mbgl::ScreenCoordinate>& anchor, const mapbox::common::optional<::mbgl::AnimationOptions>& animation);
    void pitchBy(double pitch, const mapbox::common::optional<::mbgl::AnimationOptions>& animation);
    void rotateBy(const ::mbgl::ScreenCoordinate& first, const ::mbgl::ScreenCoordinate& second, const mapbox::common::optional<::mbgl::AnimationOptions>& animation);
    ::mbgl::CameraOptions cameraForLatLngBounds(const ::mbgl::LatLngBounds& bounds, const ::mbgl::EdgeInsets& padding, const mapbox::common::optional<double>& bearing, const mapbox::common::optional<double>& pitch);
    ::mbgl::CameraOptions cameraForLatLngs(const std::vector<::mbgl::LatLng>& latlng, const ::mbgl::EdgeInsets& padding, const mapbox::common::optional<double>& bearing, const mapbox::common::optional<double>& pitch);
    ::mbgl::LatLngBounds latLngBoundsForCamera(const ::mbgl::CameraOptions& camera);
    void setProjectionMode(const ::mbgl::ProjectionMode& mode);
    ::mbgl::ProjectionMode getProjectionMode();
    void cancelTransitions();
    void setGestureInProgress(bool inProgress);
    bool isGestureInProgress();
    bool isRotating();
    bool isScaling();
    bool isPanning();
    void setBounds(const ::mbgl::BoundOptions& options);
    ::mbgl::BoundOptions getBounds();
    void setPrefetchZoomDelta(uint8_t delta);
    uint8_t getPrefetchZoomDelta();
    void setNorthOrientation(NorthOrientation orientation);
    void setConstrainMode(ConstrainMode mode);
    void setViewportMode(ViewportMode mode);
    ::mbgl::MapOptions getMapOptions();
    ::mbgl::ScreenCoordinate pixelForLatLng(const ::mbgl::LatLng& latLng);
    ::mbgl::LatLng latLngForPixel(const ::mbgl::ScreenCoordinate& pixel);
    void removeAnnotationImage(const std::string& id);
    double getTopOffsetPixelsForAnnotationImage(const std::string& id);
    uint64_t addAnnotation(const ::mbgl::Annotation& annotation);
    void updateAnnotation(uint64_t id, const ::mbgl::Annotation& annotation);
    void removeAnnotation(uint64_t id);
    void setDebug(const ::mbgl::MapDebugOptions& debugOptions);
    void cycleDebugOptions();
    ::mbgl::MapDebugOptions getDebug();
    bool isFullyLoaded();
    void dumpDebugLogs();

private:
    using Impl = MapImpl;
    std::unique_ptr<Impl> impl;

public:
    /// @cond For use only by generated bindings.
    mapbox::common::peer peer;
    /// @endcond
};

} // namespace map
} // namespace mapbox
