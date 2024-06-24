#ifndef SPINNER_STATICMESHVERTEX_HPP
#define SPINNER_STATICMESHVERTEX_HPP

#include "../MeshBuilder.hpp"
#include "../GLM.hpp"

namespace Spinner::MeshData
{
    struct StaticMeshVertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec2 UV;
        // Other required: Normal, Tangent

        static std::vector<VertexAttribute> GetVertexAttributes();
        static MeshBuilder CreateMeshBuilder();

        static MeshBuffer::Pointer CreateTestTriangle();
    };
} // Spinner

#endif //SPINNER_STATICMESHVERTEX_HPP
