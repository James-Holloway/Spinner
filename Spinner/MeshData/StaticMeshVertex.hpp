#ifndef SPINNER_STATICMESHVERTEX_HPP
#define SPINNER_STATICMESHVERTEX_HPP

#include "../DrawCommand.hpp"
#include "../MeshBuilder.hpp"
#include "../GLM.hpp"
#include "../Utilities.hpp"
#include "../Shader.hpp"

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

        // Shaders
        static Spinner::ShaderGroup::Pointer ShaderGroup;
        static Spinner::ShaderGroup::Pointer ShadowShaderGroup;
        static void CreateShaders();
        static void DestroyShaders();

        static MeshBuffer::Pointer CreateTestTriangle();
        
        static void UpdateDrawComponentCallback(const Spinner::DrawCommand::Pointer &drawCommand, Components::Component *drawComponent);
    };

#pragma pack(pop)
} // Spinner

#endif //SPINNER_STATICMESHVERTEX_HPP
