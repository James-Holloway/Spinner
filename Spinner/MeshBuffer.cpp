#include "MeshBuffer.hpp"

#include <utility>
#include "Graphics.hpp"

namespace Spinner
{
    MeshBuffer::MeshBuffer(void *vertexData, size_t vertexDataSize, MeshBuffer::IndexType *indices, uint32_t indexCount, std::vector<vk::VertexInputAttributeDescription2EXT> attributeDescriptions, vk::VertexInputBindingDescription2EXT bindingDescription) :
            Buffer(vertexDataSize + (indexCount * sizeof(IndexType)), MeshBuffer::MeshBufferUsageFlags, vma::MemoryUsage::eGpuOnly, 0, false), VertexAttributeDescriptions(std::move(attributeDescriptions)), VertexBindingDescription(bindingDescription), IndexCount(indexCount)
    {
        size_t indexSize = indexCount * sizeof(IndexType);

        VertexDataOffset = 0;
        IndexDataOffset = vertexDataSize;

        auto commandBuffer = Graphics::BeginSingleTimeCommands();

        Write(vertexData, vertexDataSize, VertexDataOffset, commandBuffer);
        Write(indices, indexSize, IndexDataOffset, commandBuffer);

        Graphics::EndSingleTimeCommands(commandBuffer);
    }
} // Spinner