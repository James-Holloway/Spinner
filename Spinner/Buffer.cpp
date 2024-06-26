#include "Buffer.hpp"

#include "Graphics.hpp"
#include <cstring>
#include "Image.hpp"

namespace Spinner
{
    Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsage, vk::DeviceSize alignment, bool mapped) : BufferSize(size), Alignment(alignment), BufferUsageFlags(usageFlags), VmaMemoryUsage(memoryUsage)
    {
        bool cpuMemoryUsage = (memoryUsage == vma::MemoryUsage::eCpuToGpu || memoryUsage == vma::MemoryUsage::eCpuCopy || memoryUsage == vma::MemoryUsage::eCpuOnly);
        if (mapped && !cpuMemoryUsage)
        {
            throw std::runtime_error("Non-CPU visible memory usage cannot be mapped to CPU");
        }

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.usage = usageFlags;
        bufferInfo.size = size;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = memoryUsage;

        if (alignment != 0)
        {
            auto pair = Graphics::GetAllocator().createBufferWithAlignment(bufferInfo, allocInfo, alignment);

            VkBuffer = pair.first;
            VmaAllocation = pair.second;
        }
        else
        {
            auto pair = Graphics::GetAllocator().createBuffer(bufferInfo, allocInfo);

            VkBuffer = pair.first;
            VmaAllocation = pair.second;
        }

        if (mapped)
        {
            Mapped = Graphics::GetAllocator().mapMemory(VmaAllocation);
            // Set buffer to 0
            std::memset(Mapped, 0, BufferSize);
        }
    }

    Buffer::~Buffer()
    {
        if (VmaAllocation)
        {
            if (Mapped != nullptr)
            {
                Graphics::GetAllocator().unmapMemory(VmaAllocation);
                Mapped = nullptr;
            }

            Graphics::GetAllocator().destroyBuffer(VkBuffer, VmaAllocation);

            VkBuffer = nullptr;
            VmaAllocation = nullptr;
        }
    }

    void Buffer::Write(const void *data, vk::DeviceSize size, vk::DeviceAddress offset, Spinner::CommandBuffer::Pointer commandBuffer)
    {
        assert((size + offset) <= BufferSize);

        if (!VmaAllocation)
        {
            throw std::runtime_error("Cannot Write to an invalid Buffer");
        }

        if (Mapped != nullptr)
        {
            std::memcpy(Mapped, data, size);
            Graphics::GetAllocator().flushAllocation(VmaAllocation, offset, size);
        }
        else
        {
            // Ensure buffer can be transferred to
            assert(static_cast<vk::BufferUsageFlags::MaskType>(BufferUsageFlags & vk::BufferUsageFlagBits::eTransferDst) != 0);

            bool singleTime = false;
            if (commandBuffer == nullptr)
            {
                commandBuffer = Graphics::BeginSingleTimeCommands();
                singleTime = true;
            }

            Buffer::Pointer stagingBuffer = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu, 0, true);
            stagingBuffer->Write(data, size, 0, nullptr);

            vk::BufferCopy copyRegion;
            copyRegion.size = size;
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = offset;

            commandBuffer->TrackObject(stagingBuffer);
            // commandBuffer->TrackObject(shared_from_this()); // Have to presume that this buffer is being referenced more permanently elsewhere. Cannot write on creation
            commandBuffer->CopyBuffer(stagingBuffer->VkBuffer, VkBuffer, copyRegion);

            if (singleTime)
            {
                Graphics::EndSingleTimeCommands(commandBuffer);
            }
        }
    }

    void Buffer::CopyTo(const Buffer::Pointer &destination, CommandBuffer::Pointer commandBuffer)
    {
        // Ensure we can transfer from this
        assert(static_cast<vk::BufferUsageFlags::MaskType>(BufferUsageFlags & vk::BufferUsageFlagBits::eTransferDst) != 0);
        // And destination buffer can be transferred to
        assert(static_cast<vk::BufferUsageFlags::MaskType>(destination->BufferUsageFlags & vk::BufferUsageFlagBits::eTransferSrc) != 0);
        // And that this buffer's size fits in other's size
        assert(BufferSize <= destination->BufferSize);

        bool singleTime = false;
        if (commandBuffer == nullptr)
        {
            commandBuffer = Graphics::BeginSingleTimeCommands();
            singleTime = true;
        }

        vk::BufferCopy copyRegion;
        copyRegion.size = BufferSize;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;

        commandBuffer->TrackObject(shared_from_this());
        commandBuffer->TrackObject(destination);
        commandBuffer->CopyBuffer(VkBuffer, destination->VkBuffer, copyRegion);

        if (singleTime)
        {
            Graphics::EndSingleTimeCommands(commandBuffer);
        }
    }

    Buffer::Pointer Buffer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsage, vk::DeviceSize alignment, bool mapped)
    {
        return std::make_shared<Buffer>(size, usageFlags, memoryUsage, alignment, mapped);
    }

    void Buffer::CopyToImage(const std::shared_ptr<Image> &image, vk::ImageAspectFlags imageAspectFlags, CommandBuffer::Pointer commandBuffer, std::optional<vk::ImageSubresourceLayers> subresourceLayers, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        // Ensure we can transfer from this
        assert(static_cast<vk::BufferUsageFlags::MaskType>(BufferUsageFlags & vk::BufferUsageFlagBits::eTransferDst) != 0);
        // And destination image can be transferred to
        assert(static_cast<vk::BufferUsageFlags::MaskType>(image->ImageUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) != 0);
        // And that this buffer's size fits in image's size
        assert(BufferSize <= image->GetImageSize());

        if (!subresourceLayers.has_value())
        {
            subresourceLayers = vk::ImageSubresourceLayers(imageAspectFlags, 0, 0, 1);
        }

        bool singleTime = false;
        if (commandBuffer == nullptr)
        {
            commandBuffer = Graphics::BeginSingleTimeCommands();
            singleTime = true;
        }

        vk::BufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageExtent = image->GetExtent();
        region.imageOffset = vk::Offset3D(0, 0, 0);
        region.imageSubresource = subresourceLayers.value();

        commandBuffer->TrackObject(shared_from_this());
        commandBuffer->TrackObject(image);

        auto oldImageLayout = image->GetCurrentImageLayout();
        auto vkImage = image->GetImage();
        commandBuffer->TransitionImageLayout(image, vk::ImageLayout::eTransferDstOptimal, imageAspectFlags, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eAllCommands, subresourceRange);
        commandBuffer->CopyBufferToImage(VkBuffer, vkImage, image->CurrentImageLayout, region);
        commandBuffer->TransitionImageLayout(image, oldImageLayout, imageAspectFlags, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eAllCommands, subresourceRange);

        if (singleTime)
        {
            Graphics::EndSingleTimeCommands(commandBuffer);
        }
    }

} // Spinner