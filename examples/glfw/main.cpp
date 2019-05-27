#include "window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

void glfwError(int error, const char* description) {
    std::cerr << "GLFW error (" << error << "): " << description << std::endl;
    exit(1);
}

int main() {
    glfwSetErrorCallback(glfwError);
    glfwInit();

#ifdef MBGL_USE_GLES2
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#endif

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    Window window;
    window.run();

    return 0;
}
