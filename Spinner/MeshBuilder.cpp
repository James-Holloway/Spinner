#include "MeshBuilder.hpp"

#include "VulkanUtilities.hpp"

namespace Spinner
{

    MeshBuilder::MeshBuilder(const std::vector<VertexAttribute> &attributes) : Attributes(attributes)
    {
        uint32_t attributeIndex = 0;
        uint32_t attributeOffset = 0;
        for (auto &attribute : Attributes)
        {
            if (attribute.Format == vk::Format::eUndefined)
            {
                throw std::runtime_error("MeshBuilder cannot initialize from a VertexAttribute which has an Undefined format");
            }
            if (attribute.Location == VertexAttribute::AutoLocation)
            {
                attribute.Location = attributeIndex;
            }
            if (attribute.Offset == VertexAttribute::AutoOffset)
            {
                attribute.Offset = attributeOffset;
            }
            else
            {
                attributeOffset = attribute.Offset;
            }
            attributeIndex++;
            attributeOffset += VkFormatByteWidth(attribute.Format);
        }

        Stride = attributeOffset;

        VertexData.resize(Attributes.size());
    }

    MeshBuilder &MeshBuilder::SetVertexData(const void *data, uint32_t vertexCount)
    {
        if (data != nullptr && vertexCount > 0)
        {
            auto vertexData = reinterpret_cast<const uint8_t *>(data);
            VertexData = {&vertexData[0], &vertexData[vertexCount]};
        }
        else
        {
            VertexData.clear();
        }

        return *this;
    }

    MeshBuilder &MeshBuilder::SetIndices(MeshBuffer::IndexType *indices, uint32_t indexCount)
    {
        if (indexCount != 0)
        {
            Indices = {&indices[0], &indices[indexCount]};
        }
        else
        {
            Indices.clear();
        }

        return *this;
    }

    MeshBuilder &MeshBuilder::SetIndices(const std::vector<MeshBuffer::IndexType> &indices)
    {
        Indices = indices;

        return *this;
    }

    MeshBuffer::Pointer MeshBuilder::Create()
    {
        std::vector<vk::VertexInputAttributeDescription2EXT> attributeDescriptions(Attributes.size(), vk::VertexInputAttributeDescription2EXT{});
        vk::VertexInputBindingDescription2EXT bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        bindingDescription.stride = Stride;
        bindingDescription.divisor = 1;

        for (size_t i = 0; i < Attributes.size(); i++)
        {
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].location = Attributes[i].Location;
            attributeDescriptions[i].format = Attributes[i].Format;
            attributeDescriptions[i].offset = Attributes[i].Offset;
        }

        return std::make_shared<MeshBuffer>(VertexData.data(), VertexData.size(), Indices.data(), static_cast<uint32_t>(Indices.size()), attributeDescriptions, bindingDescription);
    }
} // Spinner