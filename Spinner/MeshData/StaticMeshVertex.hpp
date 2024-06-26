#ifndef SPINNER_STATICMESHVERTEX_HPP
#define SPINNER_STATICMESHVERTEX_HPP

#include "../MeshBuilder.hpp"
#include "../GLM.hpp"
#include "../Utilities.hpp"

namespace Spinner::MeshData
{

#pragma pack(push, 2)

    struct StaticMeshVertex
    {
        glm::vec3 Position = {0, 0, 0};
        glm::vec3 Normal = {0, 0, 0};
        glm::vec4 Tangent = {0, 0, 0, 1}; // Handedness stored in W
        glm::vec3 Color = {1, 1, 1};
        glm::vec2 UV = {0, 0};

        // Vertex & shader descriptions
        static std::vector<VertexAttribute> GetVertexAttributes();
        static size_t GetStride();
        static MeshBuilder CreateMeshBuilder();
        static std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings();

        // Shaders & descriptor pool
        static Shader::Pointer VertexShader;
        static Shader::Pointer FragmentShader;
        static DescriptorPool::Pointer DescriptorPool;
        static void CreateShaders();
        static void DestroyShaders();

        static MeshBuffer::Pointer CreateTestTriangle();
    };

#pragma pack(pop)

} // Spinner

#endif //SPINNER_STATICMESHVERTEX_HPP
