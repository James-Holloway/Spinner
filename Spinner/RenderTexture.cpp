#include "RenderTexture.hpp"

#include <utility>

#include "Graphics.hpp"

namespace Spinner
{
    RenderTexture::RenderTexture(Spinner::Image::Pointer image, vk::ImageAspectFlags aspectFlags) : Image(std::move(image)), AspectFlags(aspectFlags)
    {
        Format = Image->GetFormat();
        if (Image->GetMainImageView() == nullptr)
        {
            Image->CreateMainImageView(aspectFlags);
        }
    }

    RenderTexture::RenderTexture(vk::Extent2D extent, vk::Format format, vk::ImageAspectFlags aspectFlags) : Format(format), AspectFlags(aspectFlags)
    {
        Recreate(extent);
    }

    void RenderTexture::Recreate(vk::Extent2D extent)
    {
        bool isDepthAspect = (AspectFlags & vk::ImageAspectFlagBits::eDepth) != vk::ImageAspectFlagBits::eNone;
        vk::ImageUsageFlags usageFlags = isDepthAspect ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment;
        Image = Image::CreateImage(extent, Format, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | usageFlags, vk::ImageType::e2D, vk::ImageTiling::eOptimal, 1, vma::MemoryUsage::eGpuOnly);
        Image->CreateMainImageView(AspectFlags);
    }


    std::string RenderTexture::GetName() const
    {
        return Name;
    }

    void RenderTexture::SetName(const std::string &name)
    {
        Name = name;
    }

    Spinner::Image::Pointer RenderTexture::GetImage() const
    {
        return Image;
    }

    vk::ImageView RenderTexture::GetMainImageView() const
    {
        if (Image == nullptr)
        {
            return nullptr;
        }
        return Image->GetMainImageView();
    }

    vk::Format RenderTexture::GetFormat() const
    {
        return Format;
    }

    vk::ImageAspectFlags RenderTexture::GetAspectFlags() const
    {
        return AspectFlags;
    }

    RenderTexture::Pointer RenderTexture::CreateRenderTexture(vk::Extent2D extent, vk::Format format, vk::ImageAspectFlags aspectFlags)
    {
        return std::make_shared<RenderTexture>(extent, format, aspectFlags);
    }

    RenderTexture::Pointer RenderTexture::CreateDepthRenderTexture(vk::Extent2D extent)
    {
        auto depthFormat = Graphics::FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD16Unorm}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        return CreateRenderTexture(extent, depthFormat, vk::ImageAspectFlagBits::eDepth);
    }

} // Spinner