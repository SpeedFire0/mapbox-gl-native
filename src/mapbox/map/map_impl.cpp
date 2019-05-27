#include <mapbox/common/optional.hpp>
#include <mapbox/map/map_impl.hpp>
#include <mapbox/map/scheduler.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/storage/default_file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/premultiply.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/traits.hpp>

#include "map_load_error.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <memory>

#ifndef GL_VIEWPORT
#define GL_VIEWPORT 0x0BA2
#endif

namespace mapbox {
namespace map {

// MapLoadError
static_assert(mbgl::underlying_type(MapLoadError::StyleLoadError) == mbgl::underlying_type(mbgl::MapLoadError::StyleLoadError), "error");
static_assert(mbgl::underlying_type(MapLoadError::StyleParseError) == mbgl::underlying_type(mbgl::MapLoadError::StyleParseError), "error");
static_assert(mbgl::underlying_type(MapLoadError::NotFoundError) == mbgl::underlying_type(mbgl::MapLoadError::NotFoundError), "error");
static_assert(mbgl::underlying_type(MapLoadError::UnknownError) == mbgl::underlying_type(mbgl::MapLoadError::UnknownError), "error");

// MapMode
static_assert(mbgl::underlying_type(MapMode::Continuous) == mbgl::underlying_type(mbgl::MapMode::Continuous), "error");
static_assert(mbgl::underlying_type(MapMode::Static) == mbgl::underlying_type(mbgl::MapMode::Static), "error");
static_assert(mbgl::underlying_type(MapMode::Tile) == mbgl::underlying_type(mbgl::MapMode::Tile), "error");

// ConstrainMode
static_assert(mbgl::underlying_type(ConstrainMode::None) == mbgl::underlying_type(mbgl::ConstrainMode::None), "error");
static_assert(mbgl::underlying_type(ConstrainMode::HeightOnly) == mbgl::underlying_type(mbgl::ConstrainMode::HeightOnly), "error");
static_assert(mbgl::underlying_type(ConstrainMode::WidthAndHeight) == mbgl::underlying_type(mbgl::ConstrainMode::WidthAndHeight), "error");

// ViewportMode
static_assert(mbgl::underlying_type(ViewportMode::Default) == mbgl::underlying_type(mbgl::ViewportMode::Default), "error");
static_assert(mbgl::underlying_type(ViewportMode::FlippedY) == mbgl::underlying_type(mbgl::ViewportMode::FlippedY), "error");

// NorthOrientation
static_assert(mbgl::underlying_type(NorthOrientation::Upwards) == mbgl::underlying_type(mbgl::NorthOrientation::Upwards), "error");
static_assert(mbgl::underlying_type(NorthOrientation::Rightwards) == mbgl::underlying_type(mbgl::NorthOrientation::Rightwards), "error");
static_assert(mbgl::underlying_type(NorthOrientation::Downwards) == mbgl::underlying_type(mbgl::NorthOrientation::Downwards), "error");
static_assert(mbgl::underlying_type(NorthOrientation::Leftwards) == mbgl::underlying_type(mbgl::NorthOrientation::Leftwards), "error");

namespace {

Scheduler* getScheduler() {
    static thread_local std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>();
    return scheduler.get();
}

}  // namespace

MapImpl::MapImpl(std::shared_ptr<MapClient> client,
                 std::shared_ptr<mapbox::map::MapObserver> observer,
                 const mbgl::MapOptions& mapOptions,
                 const mbgl::ResourceOptions& resourceOptions)
    : size_(mapOptions.size()), pixelRatio_(mapOptions.pixelRatio())
    , observer_(std::move(observer))
    , client_(std::move(client)) {
    map_ = std::make_unique<mbgl::Map>(*this, *this, mapOptions, resourceOptions);
}

MapImpl::~MapImpl() = default;

void MapImpl::createRenderer() {
    assert(!rendererBackend_);
    assert(!renderer_);

    renderThreadId_ = std::make_unique<std::thread::id>(std::this_thread::get_id());

    rendererBackend_ = std::make_unique<RendererBackend>(
        std::bind(&MapClient::getGLProcAddress, client_, std::placeholders::_1),
        mbgl::Size{uint32_t(size_.width * pixelRatio_), uint32_t(size_.height * pixelRatio_)}, defaultFramebufferID_);

    renderer_ = std::make_unique<mbgl::Renderer>(*rendererBackend_, pixelRatio_);

    renderer_->setObserver(rendererObserver_);

    schedulerNotify_ = std::make_shared<Scheduler::NotifyFn>([this] {  client_->scheduleRepaint(); });
    getScheduler()->attach(schedulerNotify_);
    mbgl::Scheduler::SetCurrent(getScheduler());
}

void MapImpl::destroyRenderer() {
    assert(*renderThreadId_ == std::this_thread::get_id());
    schedulerNotify_.reset();
    renderer_.reset();
    rendererBackend_.reset();
}

void MapImpl::render() {
    assert(!renderThreadId_ || (*renderThreadId_ == std::this_thread::get_id()));
    mbgl::gfx::BackendScope scope(*rendererBackend_, mbgl::gfx::BackendScope::ScopeType::Implicit);

    std::shared_ptr<mbgl::UpdateParameters> params;
    {
        // Lock on the parameters
        std::lock_guard<std::mutex> lock(updateMutex_);

        if (!updateParameters_) {
            return;
        }

        // Hold on to the update parameters during render
        params = updateParameters_;
    }

    if (params) {
        if (!renderer_) {
            createRenderer();
        }

        assert(*renderThreadId_ == std::this_thread::get_id());
        {
            std::lock_guard<std::mutex> lock(resizeMutex_);

            mbgl::platform::GLint viewport[4];
            mbgl::platform::glGetIntegerv(GL_VIEWPORT, viewport);
            x_ = viewport[0];
            y_ = viewport[1];
            rendererBackend_->move(x_, y_);

            if (sizeChanged_) {
                rendererBackend_->resize(mbgl::Size(static_cast<uint32_t>(size_.width * pixelRatio_),
                                                    static_cast<uint32_t>(size_.height * pixelRatio_)));
                rendererBackend_->bind();
                sizeChanged_ = false;
            }
            renderer_->render(*params);
        }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    static_cast<Scheduler*>(mbgl::Scheduler::GetCurrent())->processEvents();
}

void MapImpl::setSize(const ::mbgl::Size& size) {
    std::lock_guard<std::mutex> lock(resizeMutex_);
    size_ = size;
    sizeChanged_ = true;

    map_->setSize(size_);
}

void MapImpl::setDefaultFramebufferObject(uint32_t framebufferID) {
    defaultFramebufferID_ = framebufferID;
    if (rendererBackend_) {
        rendererBackend_->setDefaultFramebufferObject(defaultFramebufferID_);
    }
}

void MapImpl::setStyleURL(const std::string& styleURL) {
    map_->getStyle().loadURL(styleURL);
}

void MapImpl::triggerRepaint() {
    map_->triggerRepaint();
}

void MapImpl::renderStill(StillImageCallback callback) {
    map_->renderStill([cb = std::move(callback)](const std::exception_ptr eptr) {
        mapbox::common::optional<std::string> error;
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const std::exception& e) {
            error = std::string(e.what());
        }

        if (cb) {
            cb(error);
         }
    });
}

void MapImpl::renderStill(const ::mbgl::CameraOptions& camera, const ::mbgl::MapDebugOptions& options, StillImageCallback callback) {
    map_->renderStill(camera, options, [cb = std::move(callback)](const std::exception_ptr eptr) {
        mapbox::common::optional<std::string> error;
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const std::exception& e) {
            error = std::string(e.what());
        }

        if (cb) {
            cb(error);
        }
    });
}

mbgl::CameraOptions MapImpl::getCameraOptions(const mbgl::EdgeInsets& padding) const {
    return map_->getCameraOptions(padding);
}

void MapImpl::jumpTo(const mbgl::CameraOptions& camera) {
    try {
        map_->jumpTo(camera);
    } catch (const std::exception& e) {
    }
}

void MapImpl::easeTo(const mbgl::CameraOptions& camera, const mbgl::AnimationOptions& animation) {
    map_->easeTo(camera, animation);
}

void MapImpl::flyTo(const mbgl::CameraOptions& camera, const mbgl::AnimationOptions& animation) {
    map_->flyTo(camera, animation);
}

void MapImpl::moveBy(const mbgl::ScreenCoordinate& point, const std::experimental::optional<::mbgl::AnimationOptions>& animation) {
   map_->moveBy(point, animation.value_or(mbgl::AnimationOptions{}));
}

void MapImpl::scaleBy(double scale, const std::experimental::optional<mbgl::ScreenCoordinate>& anchor, const std::experimental::optional<::mbgl::AnimationOptions>& animation){
   map_->scaleBy(scale, anchor, animation.value_or(mbgl::AnimationOptions{}));
}

void MapImpl::pitchBy(double pitch, const std::experimental::optional<::mbgl::AnimationOptions>& animation) {
   map_->pitchBy(pitch, animation.value_or(mbgl::AnimationOptions{}));
}

void MapImpl::rotateBy(const mbgl::ScreenCoordinate& first, const mbgl::ScreenCoordinate& second, const std::experimental::optional<::mbgl::AnimationOptions>& animation) {
   map_->rotateBy(first, second, animation.value_or(mbgl::AnimationOptions{}));
}

mbgl::CameraOptions MapImpl::cameraForLatLngBounds(const mbgl::LatLngBounds& bounds, const mbgl::EdgeInsets& padding, const std::experimental::optional<double>& bearing, const std::experimental::optional<double>& pitch) {
   return map_->cameraForLatLngBounds(bounds, padding, bearing, pitch);
}

mbgl::CameraOptions MapImpl::cameraForLatLngs(const std::vector<mbgl::LatLng>& latlng, const mbgl::EdgeInsets& padding, const std::experimental::optional<double>& bearing, const std::experimental::optional<double>& pitch) {
   return map_->cameraForLatLngs(latlng, padding, bearing, pitch);
}

mbgl::LatLngBounds MapImpl::latLngBoundsForCamera(const mbgl::CameraOptions& camera) {
   return map_->latLngBoundsForCamera(camera);
}

void MapImpl::setProjectionMode(const mbgl::ProjectionMode& options) {
    map_->setProjectionMode(options);
}

mbgl::ProjectionMode MapImpl::getProjectionMode() const {
    return map_->getProjectionMode();
}

void MapImpl::cancelTransitions() {
    map_->cancelTransitions();
}

void MapImpl::setGestureInProgress(bool inProgress) {
    map_->setGestureInProgress(inProgress);
}

bool MapImpl::isGestureInProgress() const {
    return map_->isGestureInProgress();
}

bool MapImpl::isRotating() const {
    return map_->isRotating();
}

bool MapImpl::isScaling() const {
    return map_->isScaling();
}

bool MapImpl::isPanning() const {
    return map_->isPanning();
}

void MapImpl::setBounds(const ::mbgl::BoundOptions& options) {
    map_->setBounds(options);
}

::mbgl::BoundOptions MapImpl::getBounds() const {
    return map_->getBounds();
}

void MapImpl::setPrefetchZoomDelta(uint8_t delta) {
    map_->setPrefetchZoomDelta(delta);
}

uint8_t MapImpl::getPrefetchZoomDelta() const {
    return  map_->getPrefetchZoomDelta();
}

void MapImpl::setNorthOrientation(NorthOrientation orientation) {
    map_->setNorthOrientation(static_cast<mbgl::NorthOrientation>(orientation));
}

void MapImpl::setConstrainMode(ConstrainMode mode) {
    map_->setConstrainMode(static_cast<mbgl::ConstrainMode>(mode));
}

void MapImpl::setViewportMode(ViewportMode mode) {
    map_->setViewportMode(static_cast<mbgl::ViewportMode>(mode));
}

mbgl::MapOptions MapImpl::getMapOptions() const {
    return map_->getMapOptions();
}

mbgl::ScreenCoordinate MapImpl::pixelForLatLng(const mbgl::LatLng& latLng) {
    return map_->pixelForLatLng(latLng);
}

mbgl::LatLng MapImpl::latLngForPixel(const mbgl::ScreenCoordinate& pixel) {
    return map_->latLngForPixel(pixel);
}

void MapImpl::removeAnnotationImage(const std::string& id) {
    map_->removeAnnotationImage(id);
}

double MapImpl::getTopOffsetPixelsForAnnotationImage(const std::string& id) {
    return map_->getTopOffsetPixelsForAnnotationImage(id);
}

mbgl::AnnotationID MapImpl::addAnnotation(const mbgl::Annotation& annotation) {
    return map_->addAnnotation(annotation);
}

void MapImpl::updateAnnotation(uint64_t id, const mbgl::Annotation& annotation) {
    map_->updateAnnotation(id, annotation);
}

void MapImpl::removeAnnotation(uint64_t id) {
    map_->removeAnnotation(id);
}

void MapImpl::setDebug(const mbgl::MapDebugOptions& debugOptions) {
    map_->setDebug(debugOptions);
}

void MapImpl::cycleDebugOptions() {
     map_->cycleDebugOptions();
}

mbgl::MapDebugOptions MapImpl::getDebug() const {
    return map_->getDebug();
}

bool MapImpl::isFullyLoaded() const {
    return map_->isFullyLoaded();
}

void MapImpl::dumpDebugLogs() const {
    map_->dumpDebugLogs();
}

void MapImpl::reset() {
    assert(clientThreadId_ == std::this_thread::get_id());
}

void MapImpl::setObserver(mbgl::RendererObserver& observer) {
    rendererObserver_ = &observer;
    if (renderer_) {
        renderer_->setObserver(&observer);
    }
}

void MapImpl::update(std::shared_ptr<mbgl::UpdateParameters> updateParameters) {
    std::lock_guard<std::mutex> lock(updateMutex_);
    updateParameters_ = std::move(updateParameters);
    client_->scheduleRepaint();
}

void MapImpl::onCameraWillChange(CameraChangeMode) {}

void MapImpl::onCameraIsChanging() {}

void MapImpl::onCameraDidChange(CameraChangeMode) {}

void MapImpl::onWillStartLoadingMap() {}

void MapImpl::onDidFinishLoadingMap() {
    observer_->onMapChanged(MapChange::DidFinishLoadingMap);
}

void MapImpl::onDidFailLoadingMap(mbgl::MapLoadError error, const std::string& message) {
    observer_->onMapLoadError(static_cast<MapLoadError>(error), message);
}

void MapImpl::onWillStartRenderingFrame() {}

void MapImpl::onDidFinishRenderingFrame(RenderMode) {}

void MapImpl::onWillStartRenderingMap() {}

void MapImpl::onDidFinishRenderingMap(RenderMode) {}

void MapImpl::onDidFinishLoadingStyle() {
    observer_->onMapChanged(MapChange::DidFinishLoadingStyle);
}

void MapImpl::onSourceChanged(mbgl::style::Source&) {}

}  // namespace map
}  // namespace mapbox
