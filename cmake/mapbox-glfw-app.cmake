add_executable(mapbox-glfw-app
    ${CMAKE_SOURCE_DIR}/examples/glfw/main.cpp
)

target_sources(mapbox-glfw-app
    PRIVATE ${CMAKE_SOURCE_DIR}/examples/glfw/window.cpp
)

target_include_directories(mapbox-glfw-app
    PRIVATE platform/default/include
    PRIVATE include/mapbox/map
)

target_link_libraries(mapbox-glfw-app
    PRIVATE mbgl-core
    PRIVATE glfw
    PRIVATE cheap-ruler-cpp
    PRIVATE args
)

mapbox_glfw_app()
