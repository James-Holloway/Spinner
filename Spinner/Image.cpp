#include "Image.hpp"

#include "Graphics.hpp"
#include "VulkanUtilities.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"

namespace Spinner
{
    Image::Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage) : ImageExtent(extent), Format(format), ImageUsageFlags(usageFlags), ImageType(imageType)
    {
        vk::ImageCreateInfo createInfo;
        createInfo.imageType = ImageType;
        createInfo.extent = extent;
        createInfo.tiling = tiling;
        createInfo.mipLevels = mipLevels;
        createInfo.arrayLayers = 1;
        createInfo.format = Format;
        createInfo.initialLayout = CurrentImageLayout;
        createInfo.usage = usageFlags;
        createInfo.samples = vk::SampleCountFlagBits::e1;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = memoryUsage;

        auto pair = Graphics::GetAllocator().createImage(createInfo, allocInfo);
        VkImage = pair.first;
        VmaAllocation = pair.second;
    }

    Image::Image(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage) : Image(vk::Extent3D{extent.width, extent.height, 1}, format, usageFlags, imageType, tiling, mipLevels, memoryUsage)
    {

    }

    Image::~Image()
    {
        auto &device = Graphics::GetDevice();

        for (auto &imageView : VkImageViews)
        {
            if (imageView)
            {
                device.destroyImageView(imageView);
            }
        }
        VkImageViews.clear();

        if (VkImage)
        {
            Graphics::GetAllocator().destroyImage(VkImage, VmaAllocation);
        }
    }

    vk::Extent3D Image::GetExtent() const noexcept
    {
        return ImageExtent;
    }

    vk::Extent2D Image::GetExtent2D() const noexcept
    {
        return {ImageExtent.width, ImageExtent.height};
    }

    vk::Format Image::GetFormat() const noexcept
    {
        return Format;
    }

    vk::Image Image::GetImage() const noexcept
    {
        return VkImage;
    }

    vk::DeviceSize Image::GetImageSize() const
    {
        auto byteWidth = VkFormatByteWidth(Format);
        return ImageExtent.width * ImageExtent.height * ImageExtent.depth * byteWidth;
    }

    vk::ImageLayout Image::GetCurrentImageLayout() const noexcept
    {
        return CurrentImageLayout;
    }

    vk::ImageType Image::GetImageType() const noexcept
    {
        return ImageType;
    }

    void Image::Write(const std::vector<uint8_t> &textureData, vk::ImageAspectFlags aspectFlags, Spinner::CommandBuffer::Pointer commandBuffer)
    {
        vk::DeviceSize imageSize = GetImageSize();

        if (textureData.size() != imageSize)
        {
            throw std::runtime_error("Cannot write to image with different image size (ImageExtent * formatByteWidth) to textureData");
        }

        bool singleTime = false;
        if (commandBuffer == nullptr)
        {
            commandBuffer = Graphics::BeginSingleTimeCommands();
            singleTime = true;
        }

        auto stagingBuffer = Buffer::CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu, 0, true);

        stagingBuffer->Write(textureData, nullptr); // pass nullptr as command buffer as the staging buffer doesn't need it

        stagingBuffer->CopyToImage(shared_from_this(), aspectFlags, commandBuffer);

        if (singleTime)
        {
            Graphics::EndSingleTimeCommands(commandBuffer);
        }
    }

    vk::ImageView Image::CreateImageView(vk::ImageAspectFlags imageAspectFlags, vk::ImageViewType imageViewType, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange(imageAspectFlags, 0, 1, 0, 1);
        }

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = VkImage;
        createInfo.viewType = imageViewType;
        createInfo.format = Format;
        createInfo.subresourceRange = subresourceRange.value();
        createInfo.setComponents(vk::ComponentMapping{});

        auto imageView = Graphics::GetDevice().createImageView(createInfo);
        VkImageViews.push_back(imageView);

        return imageView;
    }

    void Image::DestroyImageView(vk::ImageView imageView)
    {
        if (MainImageView == imageView)
        {
            MainImageView = nullptr;
        }

        std::erase(VkImageViews, imageView);
        Graphics::GetDevice().destroyImageView(imageView);
    }

    vk::ImageView Image::CreateMainImageView(vk::ImageAspectFlags imageAspectFlags, vk::ImageViewType imageViewType, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        MainImageView = CreateImageView(imageAspectFlags, imageViewType, subresourceRange);
        return MainImageView;
    }

    vk::ImageView Image::GetMainImageView()
    {
        return MainImageView;
    }

    Image::Pointer Image::CreateImage(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage)
    {
        return std::make_shared<Spinner::Image>(extent, format, usageFlags, imageType, tiling, mipLevels, memoryUsage);
    }

    Image::Pointer Image::CreateImage(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage)
    {
        return std::make_shared<Spinner::Image>(extent, format, usageFlags, imageType, tiling, mipLevels, memoryUsage);
    }

    vk::ImageTiling Image::GetImageTiling() const noexcept
    {
        return ImageTiling;
    }
} // Spinner