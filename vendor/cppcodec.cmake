add_library(cppcodec INTERFACE)

target_include_directories(cppcodec SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/vendor/cppcodec
)
