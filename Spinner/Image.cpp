#include "Image.hpp"

#include <utility>
#include <stb_image.h>

#include "Graphics.hpp"
#include "VulkanUtilities.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Utilities.hpp"

namespace Spinner
{
    Image::Image(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage) : Image(vk::Extent3D{extent.width, extent.height, 1}, format, usageFlags, imageType, tiling, mipLevels, memoryUsage)
    {

    }

    Image::Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage) : ImageExtent(extent), Format(format), ImageUsageFlags(usageFlags), ImageType(imageType), ImageTiling(tiling)
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

    void Image::Write(const uint8_t *textureData, size_t textureDataSize, vk::ImageAspectFlags aspectFlags, Spinner::CommandBuffer::Pointer commandBuffer)
    {
        vk::DeviceSize imageSize = GetImageSize();

        if (textureDataSize != imageSize)
        {
            throw std::runtime_error("Cannot write to image with different image size (ImageExtent * formatByteWidth) to textureData");
        }

        bool singleTime = false;
        if (commandBuffer == nullptr)
        {
            commandBuffer = Graphics::BeginSingleTimeCommands();
            singleTime = true;
        }

        if (CurrentImageLayout == vk::ImageLayout::eUndefined)
        {
            commandBuffer->TransitionImageLayout(shared_from_this(), vk::ImageLayout::eShaderReadOnlyOptimal);
        }

        auto stagingBuffer = Buffer::CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu, 0, true);

        stagingBuffer->Write(textureData, textureDataSize, 0u, nullptr); // pass nullptr as command buffer as the staging buffer doesn't need it

        stagingBuffer->CopyToImage(shared_from_this(), aspectFlags, commandBuffer); // also tracks objects

        if (singleTime)
        {
            Graphics::EndSingleTimeCommands(commandBuffer);
        }
    }

    void Image::Write(const std::vector<uint8_t> &textureData, vk::ImageAspectFlags aspectFlags, Spinner::CommandBuffer::Pointer commandBuffer)
    {
        Write(textureData.data(), textureData.size(), aspectFlags, std::move(commandBuffer));
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

    Image::Pointer Image::CreateImage(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage)
    {
        return std::make_shared<Spinner::Image>(extent, format, usageFlags, imageType, tiling, mipLevels, memoryUsage);
    }

    Image::Pointer Image::CreateImage3D(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, vma::MemoryUsage memoryUsage)
    {
        return std::make_shared<Spinner::Image>(extent, format, usageFlags, imageType, tiling, mipLevels, memoryUsage);
    }

    vk::ImageTiling Image::GetImageTiling() const noexcept
    {
        return ImageTiling;
    }

    // Always loads as RGBA8 or RGBA16
    std::vector<uint8_t> Image::DecodeEmbeddedImageData(const std::vector<uint8_t> &data, int &width, int &height, int &channels, bool &is16Bit)
    {
        if (data.size() > std::numeric_limits<int>::max())
        {
            throw std::runtime_error("Cannot decode image data of size " + std::to_string(data.size()) + " as it is too big");
        }
        int dataSize = static_cast<int>(data.size());
        is16Bit = stbi_is_16_bit_from_memory(data.data(), dataSize);
        if (is16Bit)
        {
            auto image = stbi_load_16_from_memory(data.data(), dataSize, &width, &height, &channels, STBI_rgb_alpha);
            size_t imageSize = width * height * STBI_rgb_alpha * sizeof(stbi_us);

            std::vector<uint8_t> decodedData = {reinterpret_cast<uint8_t *>(&image[0]), reinterpret_cast<uint8_t *>(&image[imageSize - 1])};

            stbi_image_free(image);

            channels = STBI_rgb_alpha;

            return decodedData;
        }

        auto image = stbi_load_from_memory(data.data(), dataSize, &width, &height, &channels, STBI_rgb_alpha);
        size_t imageSize = width * height * STBI_rgb_alpha * sizeof(stbi_uc);

        std::vector<uint8_t> decodedData = {image[0], image[imageSize - 1]};

        stbi_image_free(image);

        channels = STBI_rgb_alpha;

        return decodedData;
    }

    Image::Pointer Image::LoadFromEmbeddedImageData(const std::vector<uint8_t> &data, int mipLevels)
    {
        if (data.size() > std::numeric_limits<int>::max())
        {
            throw std::runtime_error("Cannot decode image data of size " + std::to_string(data.size()) + " as it is too big");
        }
        int dataSize = static_cast<int>(data.size());
        bool is16Bit = stbi_is_16_bit_from_memory(data.data(), dataSize);
        int width = 0, height = 0, channels = 0;

        Image::Pointer image = nullptr;

        if (is16Bit)
        {
            auto loadedImage = stbi_load_16_from_memory(data.data(), dataSize, &width, &height, &channels, STBI_rgb_alpha);
            if (loadedImage != nullptr)
            {
                size_t imageSize = width * height * STBI_rgb_alpha * sizeof(stbi_us);

                image = CreateImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, vk::Format::eR16G16B16A16Unorm, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);
                image->Write(reinterpret_cast<uint8_t *>(loadedImage), imageSize, vk::ImageAspectFlagBits::eColor, nullptr);

                stbi_image_free(loadedImage);
            }
        }
        else
        {
            auto loadedImage = stbi_load_from_memory(data.data(), dataSize, &width, &height, &channels, STBI_rgb_alpha);
            if (loadedImage != nullptr)
            {
                size_t imageSize = width * height * STBI_rgb_alpha * sizeof(stbi_uc);

                image = CreateImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);
                image->Write(reinterpret_cast<uint8_t *>(loadedImage), imageSize, vk::ImageAspectFlagBits::eColor, nullptr);

                stbi_image_free(loadedImage);
            }
        }

        return image;
    }

    Image::Pointer Image::LoadFromTextureFile(const std::string &textureFilename, uint32_t mipLevels)
    {
        std::string texturePath = GetAssetPath(AssetType::Texture, textureFilename);

        if (!FileExists(texturePath))
        {
            throw std::runtime_error("Cannot create texture as it does not exist at " + texturePath);
        }

        Image::Pointer image;

        int width = 0, height = 0, channels = 0;

        int is16Bit = stbi_is_16_bit(texturePath.c_str());
        if (is16Bit)
        {
            // 16 bit
            stbi_us *loadedImage = stbi_load_16(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha); // get RGBA16 out
            if (loadedImage == nullptr)
            {
                throw std::runtime_error("Could not load texture from path " + texturePath);
            }

            image = Image::CreateImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, vk::Format::eR16G16B16A16Uint, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);

            auto pixels = reinterpret_cast<uint8_t *>(loadedImage);

            image->Write(pixels, image->GetImageSize(), vk::ImageAspectFlagBits::eColor, nullptr);

            stbi_image_free(loadedImage);
        }
        else
        {
            // 8 bit
            stbi_uc *loadedImage = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha); // get RGBA8 out
            if (loadedImage == nullptr)
            {
                throw std::runtime_error("Could not load texture from path " + texturePath);
            }

            image = Image::CreateImage({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);

            image->Write(loadedImage, image->GetImageSize(), vk::ImageAspectFlagBits::eColor, nullptr);

            stbi_image_free(loadedImage);
        }

        return image;
    }
} // Spinner