#include "StaticMeshVertex.hpp"

namespace Spinner::MeshData
{
    std::vector<VertexAttribute> StaticMeshVertex::GetVertexAttributes()
    {
        return std::vector<VertexAttribute>{
                VertexAttribute::CreateVec3(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Position)), // vec3 Position
                VertexAttribute::CreateVec3(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Color)), // vec3 Color
                VertexAttribute::CreateVec2(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, UV)) // vec2 UV
        };
    }

    MeshBuilder StaticMeshVertex::CreateMeshBuilder()
    {
        return MeshBuilder(GetVertexAttributes());
    }

    MeshBuffer::Pointer StaticMeshVertex::CreateTestTriangle()
    {
        std::vector<StaticMeshVertex> vertices{
                {{-0.5f, 0.5f, 0.0f}, {1, 0, 0}, {0.0f, 0.0f}}, // bottom left
                {{0.5f,  0.5f, 0.0f}, {0, 1, 0}, {1.0f, 0.0f}}, // bottom right
                {{0.0f,  -0.5f,  0.0f}, {0, 0, 1}, {0.5f, 0.5f}}, // top middle
        };

        std::vector<MeshBuffer::IndexType> indices{
                0, 1, 2
        };

        return CreateMeshBuilder().SetVertexData(vertices).SetIndices(indices).Create();
    }
}