#ifndef SPINNER_RENDERTEXTURE_HPP
#define SPINNER_RENDERTEXTURE_HPP

#include "Image.hpp"

namespace Spinner
{
    // Set AspectFlags to eDepth on creation to allow for depth rendering
    class RenderTexture
    {
    public:
        using Pointer = std::shared_ptr<RenderTexture>;

        explicit RenderTexture(Spinner::Image::Pointer image, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
        explicit RenderTexture(vk::Extent2D extent, vk::Format format = vk::Format::eR8G8B8Unorm, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);

    protected:
        std::string Name;
        Spinner::Image::Pointer Image;
        vk::ImageAspectFlags AspectFlags;
        vk::Format Format;

    public:
        void Recreate(vk::Extent2D extent);

        [[nodiscard]] std::string GetName() const;
        void SetName(const std::string &name);

        [[nodiscard]] Spinner::Image::Pointer GetImage() const;
        [[nodiscard]] vk::ImageView GetMainImageView() const;
        [[nodiscard]] vk::Format GetFormat() const;
        [[nodiscard]] vk::ImageAspectFlags GetAspectFlags() const;

    public:
        static Pointer CreateRenderTexture(vk::Extent2D extent, vk::Format format = vk::Format::eR8G8B8Unorm, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
        static Pointer CreateDepthRenderTexture(vk::Extent2D extent);
    };

} // Spinner

#endif //SPINNER_RENDERTEXTURE_HPP
