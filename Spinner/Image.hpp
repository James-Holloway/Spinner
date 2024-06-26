#ifndef SPINNER_IMAGE_HPP
#define SPINNER_IMAGE_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include <vk_mem_alloc.hpp>
#include "CommandBuffer.hpp"

namespace Spinner
{
    class CommandBuffer;

    class Buffer;

    class Image : public std::enable_shared_from_this<Image>
    {
        friend class CommandBuffer;

        friend class Buffer;

    public:
        using Pointer = std::shared_ptr<Image>;

        Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal, uint32_t mipLevels = 1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);
        Image(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal, uint32_t mipLevels = 1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);
        virtual ~Image();

    public:
        vk::Extent3D GetExtent() const noexcept;
        vk::Extent2D GetExtent2D() const noexcept;
        vk::Format GetFormat() const noexcept;
        vk::Image GetImage() const noexcept;
        vk::DeviceSize GetImageSize() const;
        vk::ImageLayout GetCurrentImageLayout() const noexcept;
        vk::ImageType GetImageType() const noexcept;
        vk::ImageTiling GetImageTiling() const noexcept;

        void Write(const std::vector<uint8_t> &textureData, vk::ImageAspectFlags aspectFlags, Spinner::CommandBuffer::Pointer commandBuffer = nullptr);

        /// @param imageAspectFlags are ignored if subresourceRange provided
        vk::ImageView CreateImageView(vk::ImageAspectFlags imageAspectFlags, vk::ImageViewType imageViewType = vk::ImageViewType::e2D, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
        void DestroyImageView(vk::ImageView imageView);

        vk::ImageView CreateMainImageView(vk::ImageAspectFlags imageAspectFlags, vk::ImageViewType imageViewType = vk::ImageViewType::e2D, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
        vk::ImageView GetMainImageView();

    protected:
        vk::Image VkImage;
        vma::Allocation VmaAllocation;
        std::vector<vk::ImageView> VkImageViews;
        vk::ImageView MainImageView;

        vk::Extent3D ImageExtent;
        vk::Format Format;
        vk::ImageUsageFlags ImageUsageFlags;
        vk::ImageType ImageType;
        vk::ImageTiling ImageTiling;

        vk::ImageLayout CurrentImageLayout = vk::ImageLayout::eUndefined;

    public:
        static Pointer CreateImage(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal, uint32_t mipLevels = 1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);
        static Pointer CreateImage(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal, uint32_t mipLevels = 1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);
    };

} // Spinner

#endif //SPINNER_IMAGE_HPP