#pragma once

#include <mapbox/map/map_client.hpp>

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>

#include <functional>

namespace mapbox {
namespace map {

class RendererBackend : public mbgl::gl::RendererBackend,
                        public mbgl::gfx::Renderable {
public:
    using GetGLProcAddressFn = std::function<MapClient::GLProcAddress(const char *)>;

    RendererBackend(GetGLProcAddressFn &&, mbgl::Size, uint32_t fbo);
    ~RendererBackend() override;

    void resize(mbgl::Size);
    void move(int32_t x, int32_t y);

    void bind();
    mbgl::Size getFramebufferSize() const;

    void setDefaultFramebufferObject(uint32_t framebufferID) { fbo_ = framebufferID; }

    // mbgl::gfx::RendererBackend implementation
    mbgl::gfx::Renderable& getDefaultRenderable() override {
        return *this;
    }

protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char *) final;
    void activate() final;
    void deactivate() final;
    void updateAssumedState() final;

private:
    GetGLProcAddressFn getProcAddressFn_;
    mbgl::Size size_;
    int32_t x_ = 0;
    int32_t y_ = 0;
    uint32_t fbo_;
};

}  // namespace map
}  // namespace mapbox
