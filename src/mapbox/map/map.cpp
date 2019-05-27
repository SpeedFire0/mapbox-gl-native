#include <mapbox/common/config.hpp>

#include "map.hpp"
#include "map_impl.hpp"

#include <utility>

namespace mapbox {
namespace map {

Map::Map(std::shared_ptr<MapClient> client, std::shared_ptr<MapObserver> observer, ::mbgl::MapOptions mapOptions, ::mbgl::ResourceOptions resourceOptions)
: impl(std::make_unique<Impl>(std::move(client), std::move(observer), std::move(mapOptions), std::move(resourceOptions)))
{}

Map::~Map() = default;

void Map::createRenderer() {
    impl->createRenderer();
}

void Map::destroyRenderer() {
    impl->destroyRenderer();
}

void Map::render() {
    impl->render();
}

void Map::setSize(const ::mbgl::Size& size) {
    impl->setSize(size);
}

void Map::setStyleURL(const std::string& styleURL) {
    impl->setStyleURL(styleURL);
}

void Map::setDefaultFramebufferObject(uint32_t framebufferId) {
    impl->setDefaultFramebufferObject(framebufferId);
}

void Map::renderStill(StillImageCallback callback) {
    impl->renderStill(std::move(callback));
}

void Map::renderStill(const ::mbgl::CameraOptions& camera, const ::mbgl::MapDebugOptions& options, StillImageCallback callback) {
    impl->renderStill(camera, options, std::move(callback));
}

void Map::triggerRepaint() {
    impl->triggerRepaint();
}

void Map::jumpTo(const ::mbgl::CameraOptions& camera) {
    impl->jumpTo(camera);
}

::mbgl::CameraOptions Map::getCameraOptions(const ::mbgl::EdgeInsets& padding) {
    return impl->getCameraOptions(padding);
}

void Map::easeTo(const ::mbgl::CameraOptions& camera, const ::mbgl::AnimationOptions& animation) {
    impl->easeTo(camera, animation);
}

void Map::flyTo(const ::mbgl::CameraOptions& camera, const ::mbgl::AnimationOptions& animation) {
    impl->flyTo(camera, animation);
}

void Map::moveBy(const ::mbgl::ScreenCoordinate& point, const mapbox::common::optional<::mbgl::AnimationOptions>& animation) {
    impl->moveBy(point, animation);
}

void Map::scaleBy(double scale, const mapbox::common::optional<::mbgl::ScreenCoordinate>& anchor, const mapbox::common::optional<::mbgl::AnimationOptions>& animation) {
    impl->scaleBy(scale, anchor, animation);
}

void Map::pitchBy(double pitch, const mapbox::common::optional<::mbgl::AnimationOptions>& animation) {
    impl->pitchBy(pitch, animation);
}

void Map::rotateBy(const ::mbgl::ScreenCoordinate& first, const ::mbgl::ScreenCoordinate& second, const mapbox::common::optional<::mbgl::AnimationOptions>& animation) {
    impl->rotateBy(first, second, animation);
}

::mbgl::CameraOptions Map::cameraForLatLngBounds(const ::mbgl::LatLngBounds& bounds, const ::mbgl::EdgeInsets& padding, const mapbox::common::optional<double>& bearing, const mapbox::common::optional<double>& pitch) {
    return impl->cameraForLatLngBounds(bounds, padding, bearing, pitch);
}

::mbgl::CameraOptions Map::cameraForLatLngs(const std::vector<::mbgl::LatLng>& latlng, const ::mbgl::EdgeInsets& padding, const mapbox::common::optional<double>& bearing, const mapbox::common::optional<double>& pitch) {
    return impl->cameraForLatLngs(latlng, padding, bearing, pitch);
}

::mbgl::LatLngBounds Map::latLngBoundsForCamera(const ::mbgl::CameraOptions& camera) {
    return impl->latLngBoundsForCamera(camera);
}

void Map::setProjectionMode(const ::mbgl::ProjectionMode& mode) {
    impl->setProjectionMode(mode);
}

::mbgl::ProjectionMode Map::getProjectionMode() {
    return impl->getProjectionMode();
}

void Map::cancelTransitions() {
    impl->cancelTransitions();
}

void Map::setGestureInProgress(bool inProgress) {
    impl->setGestureInProgress(inProgress);
}

bool Map::isGestureInProgress() {
    return impl->isGestureInProgress();
}

bool Map::isRotating() {
    return impl->isRotating();
}

bool Map::isScaling() {
    return impl->isScaling();
}

bool Map::isPanning() {
    return impl->isPanning();
}

void Map::setBounds(const ::mbgl::BoundOptions& options) {
    impl->setBounds(options);
}

::mbgl::BoundOptions Map::getBounds() {
    return impl->getBounds();
}

void Map::setPrefetchZoomDelta(uint8_t delta) {
    impl->setPrefetchZoomDelta(delta);
}

uint8_t Map::getPrefetchZoomDelta() {
    return impl->getPrefetchZoomDelta();
}

void Map::setNorthOrientation(NorthOrientation orientation) {
    impl->setNorthOrientation(orientation);
}

void Map::setConstrainMode(ConstrainMode mode) {
    impl->setConstrainMode(mode);
}

void Map::setViewportMode(ViewportMode mode) {
    impl->setViewportMode(mode);
}

::mbgl::MapOptions Map::getMapOptions() {
    return impl->getMapOptions();
}

::mbgl::ScreenCoordinate Map::pixelForLatLng(const ::mbgl::LatLng& latLng) {
    return impl->pixelForLatLng(latLng);
}

::mbgl::LatLng Map::latLngForPixel(const ::mbgl::ScreenCoordinate& pixel) {
    return impl->latLngForPixel(pixel);
}

void Map::removeAnnotationImage(const std::string& id) {
    impl->removeAnnotationImage(id);
}

double Map::getTopOffsetPixelsForAnnotationImage(const std::string& id) {
    return impl->getTopOffsetPixelsForAnnotationImage(id);
}

uint64_t Map::addAnnotation(const ::mbgl::Annotation& annotation) {
    return impl->addAnnotation(annotation);
}

void Map::updateAnnotation(uint64_t id, const ::mbgl::Annotation& annotation) {
    impl->updateAnnotation(id, annotation);
}

void Map::removeAnnotation(uint64_t id) {
    impl->removeAnnotation(id);
}

void Map::setDebug(const ::mbgl::MapDebugOptions& debugOptions) {
    impl->setDebug(debugOptions);
}

void Map::cycleDebugOptions() {
    impl->cycleDebugOptions();
}

::mbgl::MapDebugOptions Map::getDebug() {
    return impl->getDebug();
}

bool Map::isFullyLoaded() {
    return impl->isFullyLoaded();
}

void Map::dumpDebugLogs() {
    impl->dumpDebugLogs();
}

} // namespace map
} // namespace mapbox
