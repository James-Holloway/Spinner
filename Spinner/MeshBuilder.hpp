#ifndef SPINNER_MESHBUILDER_HPP
#define SPINNER_MESHBUILDER_HPP

#include "MeshBuffer.hpp"

namespace Spinner
{

    struct VertexAttribute
    {
        constexpr static uint32_t AutoLocation = 0xFFFF'FFFF;
        constexpr static uint32_t AutoOffset = 0xFFFF'FFFF;

        vk::Format Format = vk::Format::eUndefined;
        uint32_t Location = AutoLocation;
        uint32_t Offset = AutoOffset;

        constexpr static inline VertexAttribute CreateFloat(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR32Sfloat, location, offset};
        }

        constexpr static inline VertexAttribute CreateVec2(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR32G32Sfloat, location, offset};
        }

        constexpr static inline VertexAttribute CreateVec3(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR32G32B32Sfloat, location, offset};
        }

        constexpr static inline VertexAttribute CreateVec4(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR32Sfloat, location, offset};
        }

        constexpr static inline VertexAttribute CreateUint(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR32Uint, location, offset};
        }

        constexpr static inline VertexAttribute CreateUshort(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR16Uint, location, offset};
        }

        constexpr static inline VertexAttribute CreateByte(uint32_t location, uint32_t offset)
        {
            return VertexAttribute{vk::Format::eR8Uint, location, offset};
        }
    };

    class MeshBuilder
    {
    public:
        explicit MeshBuilder(const std::vector<VertexAttribute> &attributes);

    public:
        MeshBuilder &SetVertexData(const void *data, uint32_t vertexCount);

        template<typename T>
        inline MeshBuilder &SetVertexData(const std::vector<T> &vertexData)
        {
            if (!vertexData.empty())
            {
                return SetVertexData(reinterpret_cast<const uint8_t *>(vertexData.data()), static_cast<uint32_t>(vertexData.size() * sizeof(vertexData[0])));
            }
            return SetVertexData(nullptr, 0);
        }

        MeshBuilder &SetIndices(MeshBuffer::IndexType *indices, uint32_t indexCount);
        MeshBuilder &SetIndices(const std::vector<MeshBuffer::IndexType> &indices);
        MeshBuffer::Pointer Create();

    protected:
        std::vector<VertexAttribute> Attributes;
        std::vector<uint8_t> VertexData;
        std::vector<MeshBuffer::IndexType> Indices;
        uint32_t Stride;
    };

} // Spinner

#endif //SPINNER_MESHBUILDER_HPP
