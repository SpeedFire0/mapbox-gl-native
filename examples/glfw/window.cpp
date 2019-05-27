#include "window.hpp"

#include <mbgl/map/map_options.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/optional.hpp>

#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>

std::string getEnvVar(const std::string& var) {
    const char* value = getenv(var.c_str());
    if (value != nullptr) {
        return value;
    }

    return "";
}

Window::Window() : window_(glfwCreateWindow(mapSize_.width, mapSize_.height, "Mapbox GLFW lite", nullptr, nullptr)) {
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, onKey);
    glfwSetCursorPosCallback(window_, onMouseMove);
    glfwSetMouseButtonCallback(window_, onMouseClick);
    glfwSetScrollCallback(window_, onScroll);
    glfwSetWindowSizeCallback(window_, onWindowResize);

    // Get the FB size to determine the pixel ratio
    int fbWidth = 0;
    int fbHeight = 0;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    float aspectRatio_ = float(fbWidth) / float(mapSize_.width);
    assert(aspectRatio_ == float(fbHeight) / float(mapSize_.height));

    std::string accessToken;
    accessToken.append(getEnvVar("MAPBOX_ACCESS_TOKEN"));
    if (accessToken.empty()) {
        std::cerr << "No MAPBOX_ACCESS_TOKEN set" << std::endl;
    }

    mbgl::MapOptions mapOptions;
    mapOptions.withSize(mapSize_).withPixelRatio(aspectRatio_);

    mbgl::ResourceOptions resourceOptions;
    resourceOptions.withAccessToken(accessToken)
        .withCachePath("/tmp/mbgl-cache.db");

    map_ = std::make_unique<mapbox::map::Map>(std::make_shared<MapClient>(*this),
        std::make_shared<Observer>(), std::move(mapOptions), std::move(resourceOptions));
    map_->setStyleURL("mapbox://styles/mapbox/streets-v11");

    map_->jumpTo(mbgl::CameraOptions()
        .withCenter(mbgl::LatLng {60.170448, 24.942046})
        .withZoom(10));
}

Window::~Window() {
    map_.reset();
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::render() {
    if (needsRepaint_) {
        needsRepaint_ = false;
        map_->render();
        glfwSwapBuffers(window_);
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_) == 1;
}

void Window::scheduleRepaint() {
    needsRepaint_ = true;
    glfwPostEmptyEvent();
}

void Window::onKey(GLFWwindow* glfwWindow, int key, int /*scancode*/, int action, int mods) {
    auto* window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));

    // Handle key release events only
    if (action != GLFW_RELEASE) {
        return;
    }

    switch (key) {
        case GLFW_KEY_ESCAPE: {
            glfwSetWindowShouldClose(glfwWindow, 1);
            glfwPostEmptyEvent();
        } break;
        case GLFW_KEY_X:
            if (!mods)
                window->map_->jumpTo(mbgl::CameraOptions().withCenter(mbgl::LatLng {}).withZoom(0.0).withBearing(0.0).withPitch(0.0));
            break;
        default:
            break;
    }
}

void Window::onMouseMove(GLFWwindow* glfwWindow, double x, double y) {
    auto *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
    if (window->tracking_) {
        const double dx = x - window->lastX_;
        const double dy = y - window->lastY_;
        if (dx || dy) {
            window->map_->moveBy(mbgl::ScreenCoordinate { dx, dy }, mbgl::nullopt);
        }
    } else if (window->rotating_) {
        window->map_->rotateBy({ window->lastX_, window->lastY_ }, { x, y }, mbgl::nullopt);
    } else if (window->pitching_) {
        const double dy = y - window->lastY_;
        if (dy) {
            window->map_->pitchBy(dy / 2, mbgl::nullopt);
        }
    }
    window->lastX_ = x;
    window->lastY_ = y;
}

void Window::onMouseClick(GLFWwindow* glfwWindow, int button, int action, int modifiers) {
    auto *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));

    if (button == GLFW_MOUSE_BUTTON_RIGHT ||
        (button == GLFW_MOUSE_BUTTON_LEFT && modifiers & GLFW_MOD_CONTROL)) {
        window->rotating_ = action == GLFW_PRESS;
        window->map_->setGestureInProgress(window->rotating_);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && (modifiers & GLFW_MOD_SHIFT)) {
        window->pitching_ = action == GLFW_PRESS;
        window->map_->setGestureInProgress(window->pitching_);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        window->tracking_ = action == GLFW_PRESS;
        window->map_->setGestureInProgress(window->tracking_);

        if (action == GLFW_RELEASE) {
            double now = glfwGetTime();
            if (now - window->lastClick_ < 0.4 /* ms */) {
                if (modifiers & GLFW_MOD_SHIFT) {
                    window->map_->scaleBy(0.5, mbgl::ScreenCoordinate { window->lastX_, window->lastY_ }, mbgl::AnimationOptions{{mbgl::Milliseconds(500)}});
                } else {
                    window->map_->scaleBy(2.0, mbgl::ScreenCoordinate { window->lastX_, window->lastY_ }, mbgl::AnimationOptions{{mbgl::Milliseconds(500)}});
                }
            }
            window->lastClick_ = now;
        }
    }
}

void Window::onWindowResize(GLFWwindow* glfwWindow, int width, int height) {
    auto* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

    window->mapSize_ = mbgl::Size{uint32_t(width), uint32_t(height)};
    window->map_->setSize(window->mapSize_);
}

void Window::onScroll(GLFWwindow* glfwWindow, double /*xoffset*/, double yOffset) {
    auto *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
    double delta = yOffset * 40;

    bool isWheel = delta != 0 && std::fmod(delta, 4.000244140625) == 0;

    double absDelta = delta < 0 ? -delta : delta;
    double scale = 2.0 / (1.0 + std::exp(-absDelta / 100.0));

    // Make the scroll wheel a bit slower.
    if (!isWheel) {
        scale = (scale - 1.0) / 2.0 + 1.0;
    }

    // Zooming out.
    if (delta < 0 && scale != 0) {
        scale = 1.0 / scale;
    }

    window->map_->scaleBy(scale, mbgl::ScreenCoordinate { window->lastX_, window->lastY_ }, mbgl::nullopt);
}

MapClient::MapClient(Window& window) : window_(window) {}
MapClient::~MapClient() = default;

void MapClient::scheduleRepaint() {
    window_.scheduleRepaint();
}

mapbox::map::MapClient::GLProcAddress MapClient::getGLProcAddress(const char* name) {
    return glfwGetProcAddress(name);
}

void Window::run() {
    auto callback = [&] {
        if (glfwWindowShouldClose(window_)) {
            runLoop.stop();
            return;
        }

        glfwPollEvents();
        render();
    };

    frameTick.start(mbgl::Duration::zero(), mbgl::Milliseconds(1000 / 60), callback);
    runLoop.run();
}
