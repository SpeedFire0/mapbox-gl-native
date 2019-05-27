#include <mapbox/map/renderer_backend.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/renderable_resource.hpp>

namespace mapbox {
namespace map {

class RenderableResource final : public mbgl::gl::RenderableResource {
public:
    RenderableResource(RendererBackend& backend_) : backend(backend_) {}

    void bind() override {
        backend.bind();
    }

private:
    RendererBackend& backend;
};

RendererBackend::RendererBackend(GetGLProcAddressFn&& getProcAddressFn, mbgl::Size fbSize, uint32_t fbo)
    : mbgl::gl::RendererBackend(mbgl::gfx::ContextMode::Shared)
    , mbgl::gfx::Renderable(fbSize, std::make_unique<RenderableResource>(*this))
    , getProcAddressFn_(std::move(getProcAddressFn)), size_(fbSize), fbo_(fbo) {}

RendererBackend::~RendererBackend() = default;

void RendererBackend::resize(mbgl::Size fbSize) {
    size_ = fbSize;
}

void RendererBackend::move(int32_t x, int32_t y) {
    x_ = x;
    y_ = y;
}

void RendererBackend::updateAssumedState() {
    assumeFramebufferBinding(ImplicitFramebufferBinding);
    assumeViewport(x_, y_, getFramebufferSize());
}

void RendererBackend::bind() {
    assert(mbgl::gfx::BackendScope::exists());
    setFramebufferBinding(fbo_);
    setViewport(x_, y_, size_);
}

mbgl::Size RendererBackend::getFramebufferSize() const {
    return size_;
}

mbgl::gl::ProcAddress RendererBackend::getExtensionFunctionPointer(const char* name) {
    return getProcAddressFn_(name);
}

void RendererBackend::activate() {}

void RendererBackend::deactivate() {}

}  // namespace map
}  // namespace mapbox
