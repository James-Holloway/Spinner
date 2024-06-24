#ifndef SPINNER_MESHBUFFER_HPP
#define SPINNER_MESHBUFFER_HPP

#include "Buffer.hpp"

namespace Spinner
{
    class CommandBuffer;

    class MeshBuffer : public Buffer
    {
        friend class CommandBuffer;

    public:
        using Pointer = std::shared_ptr<MeshBuffer>;
        using IndexType = uint32_t;

        static constexpr vk::BufferUsageFlags MeshBufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

        MeshBuffer(void *vertexData, size_t vertexDataSize, IndexType *indices, uint32_t indexCount, std::vector<vk::VertexInputAttributeDescription2EXT> attributeDescriptions, vk::VertexInputBindingDescription2EXT bindingDescription);
        ~MeshBuffer() override = default;

    public:
        std::vector<vk::VertexInputAttributeDescription2EXT> VertexAttributeDescriptions;
        vk::VertexInputBindingDescription2EXT VertexBindingDescription;
        uint32_t IndexCount;
        size_t VertexDataOffset;
        size_t IndexDataOffset;
    };

} // Spinner

#endif //SPINNER_MESHBUFFER_HPP
