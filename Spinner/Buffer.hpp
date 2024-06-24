#ifndef SPINNER_BUFFER_HPP
#define SPINNER_BUFFER_HPP

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <memory>
#include <optional>

#include "CommandBuffer.hpp"

namespace Spinner
{

    class Buffer : public std::enable_shared_from_this<Buffer>
    {
    public:
        using Pointer = std::shared_ptr<Buffer>;

        Buffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsage, vk::DeviceSize alignment = 0, bool mapped = false);
        virtual ~Buffer();

    public:
        /// Writes data to the buffer directly if cpu accessible, otherwise via a staging buffer. Staging requires this buffer to have TransferDst usage flags
        void Write(void *data, vk::DeviceSize size, vk::DeviceAddress offset = 0, Spinner::CommandBuffer::Pointer commandBuffer = nullptr);
        /// Copies the buffer to another buffer. Requires this buffer to have TransferSrc and destination buffer to have TransferDst usage flags
        void CopyTo(const Buffer::Pointer &destination, CommandBuffer::Pointer commandBuffer = nullptr);

    public:
        vk::Buffer VkBuffer;
        vma::Allocation VmaAllocation;
        vk::DeviceSize BufferSize;
        vk::DeviceSize Alignment;

        vk::BufferUsageFlags BufferUsageFlags;
        vma::MemoryUsage VmaMemoryUsage;

        void *Mapped = nullptr;

    public:
        static Pointer CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsage, vk::DeviceSize alignment = 0, bool mapped = false);
    };

} // Spinner

#endif //SPINNER_BUFFER_HPP
