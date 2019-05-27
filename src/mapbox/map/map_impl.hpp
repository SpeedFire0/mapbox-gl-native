#pragma once

#include <mapbox/map/map_client.hpp>
#include <mapbox/map/renderer_backend.hpp>
#include <mapbox/map/scheduler.hpp>

#include <mbgl/annotation/annotation.hpp>
#include <mbgl/map/bound_options.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/projection_mode.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/renderer/renderer_frontend.hpp>
#include <mbgl/util/size.hpp>

#include "constrain_mode.hpp"
#include "map_mode.hpp"
#include "map_observer.hpp"
#include "north_orientation.hpp"
#include "renderer_backend.hpp"
#include "still_image_callback.hpp"
#include "viewport_mode.hpp"

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace mbgl {
class Map;
class Renderer;
class RendererObserver;
class UpdateParameters;

namespace style {
class Layer;
}  //namespace style

namespace util {
class RunLoop;
}  // namespace util
}  // namespace mbgl

namespace mapbox {
namespace map {

class MapImpl : public mbgl::RendererFrontend, public mbgl::MapObserver {
public:
    MapImpl(std::shared_ptr<MapClient> client,
            std::shared_ptr<mapbox::map::MapObserver> observer,
            const mbgl::MapOptions& mapOptions,
            const mbgl::ResourceOptions& resourceOptions);
    ~MapImpl() override;

    // Called from the render thread - these functions are _not_ thread-safe.
    void createRenderer();
    void destroyRenderer();
    void render();
    void setSize(const mbgl::Size& size);
    void setDefaultFramebufferObject(uint32_t);

    void renderStill(StillImageCallback callback);
    void renderStill(const mbgl::CameraOptions& camera, const mbgl::MapDebugOptions& options, StillImageCallback callback);

    // Triggers a repaint.
    void triggerRepaint();

    void jumpTo(const mbgl::CameraOptions& camera);
    mbgl::CameraOptions getCameraOptions(const mbgl::EdgeInsets& padding) const;

    void easeTo(const mbgl::CameraOptions& camera, const mbgl::AnimationOptions& animation);
    void flyTo(const mbgl::CameraOptions& camera, const mbgl::AnimationOptions& animation);
    void moveBy(const mbgl::ScreenCoordinate& point, const std::experimental::optional<::mbgl::AnimationOptions>& animation);
    void scaleBy(double scale, const std::experimental::optional<mbgl::ScreenCoordinate>& anchor, const std::experimental::optional<::mbgl::AnimationOptions>& animation);
    void pitchBy(double pitch, const std::experimental::optional<::mbgl::AnimationOptions>& animation);
    void rotateBy(const mbgl::ScreenCoordinate& first, const mbgl::ScreenCoordinate& second, const std::experimental::optional<::mbgl::AnimationOptions>& animation);
    mbgl::CameraOptions cameraForLatLngBounds(const mbgl::LatLngBounds& bounds, const mbgl::EdgeInsets& padding, const std::experimental::optional<double>& bearing, const std::experimental::optional<double>& pitch);
    mbgl::CameraOptions cameraForLatLngs(const std::vector<mbgl::LatLng>& latlng, const mbgl::EdgeInsets& padding, const std::experimental::optional<double>& bearing, const std::experimental::optional<double>& pitch);
    mbgl::LatLngBounds latLngBoundsForCamera(const mbgl::CameraOptions& cameraOptions);

    void setProjectionMode(const mbgl::ProjectionMode& mode);
    mbgl::ProjectionMode getProjectionMode() const;

    // Transition
    void cancelTransitions();
    void setGestureInProgress(bool);
    bool isGestureInProgress() const;
    bool isRotating() const;
    bool isScaling() const;
    bool isPanning() const;

    void setBounds(const mbgl::BoundOptions& options);
    /// Returns the current map bound options. All optional fields in BoundOptions are set.
    mbgl::BoundOptions getBounds() const;

    // Tile prefetching
    void setPrefetchZoomDelta(uint8_t delta);
    uint8_t getPrefetchZoomDelta() const;

    // Map options
    void setNorthOrientation(NorthOrientation orientation);
    void setConstrainMode(ConstrainMode mode);
    void setViewportMode(ViewportMode mode);
    mbgl::MapOptions getMapOptions() const;

    // Thread safe
    void setStyleURL(const std::string& styleURL);

    // Projection
    mbgl::ScreenCoordinate pixelForLatLng(const mbgl::LatLng& latLng);
    mbgl::LatLng latLngForPixel(const mbgl::ScreenCoordinate& pixel);

    // Annotations
    void removeAnnotationImage(const std::string& id);
    double getTopOffsetPixelsForAnnotationImage(const std::string& id);

    uint64_t addAnnotation(const mbgl::Annotation& annotation);
    void updateAnnotation(uint64_t id, const mbgl::Annotation& annotation);
    void removeAnnotation(uint64_t id);

    // Debug
    void setDebug(const mbgl::MapDebugOptions& debugOptions);
    void cycleDebugOptions();
    mbgl::MapDebugOptions getDebug() const;

    bool isFullyLoaded() const;
    void dumpDebugLogs() const;

private:
    // mbgl::RendererFrontend implementation.
    void reset() final;
    void setObserver(mbgl::RendererObserver& observer) final;
    void update(std::shared_ptr<mbgl::UpdateParameters>) final;

    // mbgl::MapObserver implementation.
    void onCameraWillChange(CameraChangeMode) final;
    void onCameraIsChanging() final;
    void onCameraDidChange(CameraChangeMode) final;
    void onWillStartLoadingMap() final;
    void onDidFinishLoadingMap() final;
    void onDidFailLoadingMap(mbgl::MapLoadError, const std::string&) final;
    void onWillStartRenderingFrame() final;
    void onDidFinishRenderingFrame(RenderMode) final;
    void onWillStartRenderingMap() final;
    void onDidFinishRenderingMap(RenderMode) final;
    void onDidFinishLoadingStyle() final;
    void onSourceChanged(mbgl::style::Source&) final;

    std::mutex resizeMutex_;
    mbgl::Size size_;
    bool sizeChanged_ = true;
    double pixelRatio_;

    std::thread::id clientThreadId_ = std::this_thread::get_id();
    std::unique_ptr<std::thread::id> renderThreadId_;

    mutable std::mutex updateMutex_;
    std::shared_ptr<mbgl::UpdateParameters> updateParameters_;
    std::unique_ptr<mbgl::Map> map_;

    // Render thread
    std::unique_ptr<RendererBackend> rendererBackend_;
    std::unique_ptr<mbgl::Renderer> renderer_;

    mbgl::RendererObserver* rendererObserver_ = nullptr;
    uint32_t defaultFramebufferID_{0};
    int32_t x_ = 0;
    int32_t y_ = 0;

    std::shared_ptr<mapbox::map::MapObserver> observer_;
    std::shared_ptr<MapClient> client_;

    std::shared_ptr<Scheduler::NotifyFn> schedulerNotify_{};
};

}  // namespace map
}  // namespace mapbox
