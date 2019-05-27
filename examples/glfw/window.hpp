#pragma once

#include "map.hpp"
#include "mapbox/map/map_client.hpp"
#include "map_observer.hpp"
#include "map_change.hpp"
#include "map_load_error.hpp"

#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/timer.hpp>

#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

struct GLFWwindow;

class Observer : public mapbox::map::MapObserver {
    void onMapChanged(mapbox::map::MapChange change) override {
        if (change == mapbox::map::MapChange::DidFinishLoadingMap)
            std::cout << "Map loading finished." << std::endl;
        else if (change == mapbox::map::MapChange::DidFinishLoadingStyle)
            std::cout << "Style loading finished." << std::endl;
    }

    void onMapLoadError(mapbox::map::MapLoadError, const std::string& message) override {
        std::cout << "Error: " << message << std::endl;
    }
};

class Window {
public:
    Window();
    virtual ~Window();

    void render();
    void scheduleRepaint();
    bool shouldClose() const;
    void run();

private:
    bool needsRepaint_ = true;
    mbgl::Size mapSize_{1024, 768};

    // GLFW callbacks.
    static void onKey(GLFWwindow *glfwWindow, int key, int, int, int);
    static void onWindowResize(GLFWwindow *glfwWindow, int width, int height);
    static void onMouseMove(GLFWwindow *glfwWindow, double x, double y);
    static void onMouseClick(GLFWwindow *glfwWindow, int button, int action, int modifiers);
    static void onScroll(GLFWwindow *glfwWindow, double xoffset, double yOffset);

    bool tracking_ = false;
    bool rotating_ = false;
    bool pitching_ = false;

    double lastX_ = 0, lastY_ = 0;

    double lastClick_ = -1;

    GLFWwindow *window_;
    mbgl::util::RunLoop runLoop;
    mbgl::util::Timer frameTick;

    std::unique_ptr<mapbox::map::Map> map_;
};

class MapClient : public mapbox::map::MapClient {
public:
    MapClient(Window& window);
    ~MapClient() override;

    void scheduleRepaint() final;
    mapbox::map::MapClient::GLProcAddress getGLProcAddress(const char *name) final;

private:
    Window& window_;
};
