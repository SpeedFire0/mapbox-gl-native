#pragma once

#if __cplusplus >= 201703L
#define WITH_OPTIONALS
#endif

#if defined(WITH_OPTIONALS)
#include <optional>
#else
#include <experimental/optional>
#endif

namespace mapbox {
namespace common {

#if defined(WITH_OPTIONALS)
using nullopt_t = std::nullopt_t;
constexpr nullopt_t nullopt = std::nullopt;
template<class T> using optional = std::optional<T>;
#else
using nullopt_t = std::experimental::nullopt_t;
constexpr nullopt_t nullopt = std::experimental::nullopt;
template<class T> using optional = std::experimental::optional<T>;
#endif

} // common
} // mapbox
