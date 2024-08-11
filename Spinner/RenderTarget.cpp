#include "RenderTarget.hpp"

namespace Spinner
{

    RenderTarget::RenderTarget(vk::Extent2D extent, bool createColor, bool createDepth, vk::Format colorFormat)
    {
        if (createColor)
        {
            ColorTexture = RenderTexture::CreateRenderTexture(extent, colorFormat);
        }
        if (createDepth)
        {
            DepthTexture = RenderTexture::CreateDepthRenderTexture(extent);
        }
    }

    RenderTarget::RenderTarget(RenderTexture::Pointer colorTexture, RenderTexture::Pointer depthTexture)
    {
        ColorTexture = std::move(colorTexture);
        DepthTexture = std::move(depthTexture);
    }

    RenderTexture::Pointer RenderTarget::GetColorTexture() const
    {
        return ColorTexture;
    }

    RenderTexture::Pointer RenderTarget::GetDepthTexture() const
    {
        return DepthTexture;
    }

    void RenderTarget::Recreate(vk::Extent2D newExtent)
    {
        if (ColorTexture != nullptr)
        {
            ColorTexture->Recreate(newExtent);
        }
        if (DepthTexture != nullptr)
        {
            DepthTexture->Recreate(newExtent);
        }
    }

    void RenderTarget::TransitionForRendering(CommandBuffer::Pointer &commandBuffer)
    {
        if (ColorTexture != nullptr && ColorTexture->GetImage() != nullptr)
        {
            auto image = ColorTexture->GetImage();
            commandBuffer->TrackObject(ColorTexture);
            commandBuffer->InsertImageMemoryBarrier(image->GetImage(), vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            commandBuffer->TransitionImageLayout(image, vk::ImageLayout::eAttachmentOptimal, ColorTexture->GetAspectFlags());
        }
        if (DepthTexture != nullptr && DepthTexture->GetImage() != nullptr)
        {
            auto image = DepthTexture->GetImage();
            commandBuffer->TrackObject(DepthTexture);
            commandBuffer->TransitionImageLayout(image, vk::ImageLayout::eAttachmentOptimal, DepthTexture->GetAspectFlags());
        }
    }

    void RenderTarget::TransitionForRead(CommandBuffer::Pointer &commandBuffer)
    {
        if (ColorTexture != nullptr && ColorTexture->GetImage() != nullptr)
        {
            auto image = ColorTexture->GetImage();
            commandBuffer->TransitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, ColorTexture->GetAspectFlags());
        }
        if (DepthTexture != nullptr && DepthTexture->GetImage() != nullptr)
        {
            auto image = DepthTexture->GetImage();
            commandBuffer->TransitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, DepthTexture->GetAspectFlags());
        }
    }

    RenderTarget::Pointer RenderTarget::CreateRenderTarget(vk::Extent2D extent, bool createColor, bool createDepth, vk::Format colorFormat)
    {
        return std::make_shared<RenderTarget>(extent, createColor, createDepth, colorFormat);
    }
} // Spinner