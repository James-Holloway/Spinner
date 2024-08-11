#ifndef SPINNER_RENDERTARGET_HPP
#define SPINNER_RENDERTARGET_HPP

#include "RenderTexture.hpp"

namespace Spinner
{
    class RenderTarget
    {
    public:
        using Pointer = std::shared_ptr<RenderTarget>;
        constexpr static vk::Format DefaultColorFormat = vk::Format::eR8G8B8A8Unorm;

        RenderTarget(vk::Extent2D extent, bool createColor, bool createDepth, vk::Format colorFormat = DefaultColorFormat);
        RenderTarget(RenderTexture::Pointer colorTexture, RenderTexture::Pointer depthTexture);

    protected:
        RenderTexture::Pointer ColorTexture = nullptr;
        RenderTexture::Pointer DepthTexture = nullptr;

    public:
        [[nodiscard]] RenderTexture::Pointer GetColorTexture() const;
        [[nodiscard]] RenderTexture::Pointer GetDepthTexture() const;

        void Recreate(vk::Extent2D newExtent);
        void TransitionForRendering(CommandBuffer::Pointer &commandBuffer);
        void TransitionForRead(CommandBuffer::Pointer &commandBuffer);

    public:
        static Pointer CreateRenderTarget(vk::Extent2D extent, bool createColor, bool createDepth, vk::Format colorFormat = DefaultColorFormat);
    };
} // Spinner

#endif //SPINNER_RENDERTARGET_HPP
