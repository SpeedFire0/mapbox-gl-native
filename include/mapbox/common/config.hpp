#pragma once

// Compiler defines for making symbols visible, otherwise they
// will be defined as hidden by default.

// clang-format off

#if defined WIN32
    #ifdef MAPBOX_MAP_BUILDING_LIB
        #ifdef __GNUC__
            #define MAPBOX_MAP_PUBLIC __attribute__((dllexport))
        #else
            #define MAPBOX_MAP_PUBLIC __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define MAPBOX_MAP_PUBLIC __attribute__((dllimport))
        #else
            #define MAPBOX_MAP_PUBLIC __declspec(dllimport)
        #endif
    #endif
#else
    #define MAPBOX_MAP_PUBLIC __attribute__((visibility ("default")))
#endif

// clang-format on
