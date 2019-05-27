#pragma once

namespace mapbox {
namespace map {

/**
* @brief Interface that must be implement by a Map client.
*/
class MapClient {
public:
    /**
     * @brief Alias for an opaque function pointer.
     */
     using GLProcAddress = void (*)();

    /**
     * @brief Returns the GL function pointer address for a given name.
     *
     * @param name a string containing a GL function name.
     * @return GLProcAddress an opaque function pointer for a GL function.
     */
    virtual GLProcAddress getGLProcAddress(const char* name) = 0;

    /**
     * @brief Notifies the client to schedule a repaint. This must eventually trigger a Map::render() call,
     * otherwise the map will be left in incomplete state.
     */
    virtual void scheduleRepaint() = 0;

protected:
    virtual ~MapClient() = default;
};

} // namespace map
} // namespace mapbox
