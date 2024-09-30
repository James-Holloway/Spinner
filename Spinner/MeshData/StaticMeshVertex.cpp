#include "StaticMeshVertex.hpp"

#include <tiny_gltf.h>

#include "../Lighting.hpp"
#include "../Shader.hpp"
#include "../Scene.hpp"

namespace Spinner::MeshData
{
    ShaderGroup::Pointer StaticMeshVertex::ShaderGroup;
    DescriptorPool::Pointer StaticMeshVertex::DescriptorPool;

    std::vector<VertexAttribute> StaticMeshVertex::GetVertexAttributes()
    {
        return std::vector<VertexAttribute>{
            VertexAttribute::CreateVec3(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Position)), // vec3 Position
            VertexAttribute::CreateVec3(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Normal)), // vec3 Normal
            VertexAttribute::CreateVec4(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Tangent)), // vec3 Tangent
            VertexAttribute::CreateVec3(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, Color)), // vec3 Color
            VertexAttribute::CreateVec2(VertexAttribute::AutoLocation, offsetof(StaticMeshVertex, UV)), // vec2 UV
        };
    }

    size_t StaticMeshVertex::GetStride()
    {
        return sizeof(StaticMeshVertex);
    }

    MeshBuilder StaticMeshVertex::CreateMeshBuilder()
    {
        return MeshBuilder(GetVertexAttributes(), GetStride());
    }

    std::vector<vk::DescriptorSetLayoutBinding> StaticMeshVertex::GetDescriptorSetLayoutBindings()
    {
        return std::vector<vk::DescriptorSetLayoutBinding>{
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
            vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr),
        };
    }

    void StaticMeshVertex::CreateShaders()
    {
        // Descriptor Pool
        DescriptorPool = DescriptorPool::CreateDefault();

        // Descriptor Set Layout and push constants
        auto descriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(GetDescriptorSetLayoutBindings(), {});

        // Scene
        auto sceneDescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(Spinner::Scene::GetDescriptorSetLayoutBindings(), {});

        // Lighting
        auto lightingDescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(Lighting::GetDescriptorSetLayoutBindings(), {}, Lighting::GetDescriptorBindingFlags());

        // Shader creation
        ShaderCreateInfo vertexShaderCreateInfo;
        vertexShaderCreateInfo.ShaderStage = vk::ShaderStageFlagBits::eVertex;
        vertexShaderCreateInfo.ShaderName = "staticmesh";
        vertexShaderCreateInfo.NextStage = vk::ShaderStageFlagBits::eFragment;
        vertexShaderCreateInfo.DescriptorSetLayouts = {descriptorSetLayout};
        vertexShaderCreateInfo.SceneDescriptorSetLayout = sceneDescriptorSetLayout;
        vertexShaderCreateInfo.LightingDescriptorSetLayout = lightingDescriptorSetLayout;

        ShaderCreateInfo fragmentShaderCreateInfo;
        fragmentShaderCreateInfo.ShaderStage = vk::ShaderStageFlagBits::eFragment;
        fragmentShaderCreateInfo.ShaderName = "staticmesh";
        fragmentShaderCreateInfo.NextStage = {};
        fragmentShaderCreateInfo.DescriptorSetLayouts = {descriptorSetLayout};
        fragmentShaderCreateInfo.SceneDescriptorSetLayout = sceneDescriptorSetLayout;
        fragmentShaderCreateInfo.LightingDescriptorSetLayout = lightingDescriptorSetLayout;

        auto shaderGroup = ShaderGroup::CreateShaderGroup({vertexShaderCreateInfo, fragmentShaderCreateInfo});

        ShaderGroup = shaderGroup;
    }

    void StaticMeshVertex::DestroyShaders()
    {
        ShaderGroup.reset();
        DescriptorPool.reset();
    }

    MeshBuffer::Pointer StaticMeshVertex::CreateTestTriangle()
    {
        std::vector<StaticMeshVertex> vertices{
            {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0, 0, 1}, {1, 0, 0}, {0.0f, 0.0f}}, // bottom left
            {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0, 0, 1}, {0, 1, 0}, {1.0f, 0.0f}}, // bottom right
            {{0.0f, -0.5f, 0.0f}, {0, 0, -1}, {0, 0, 0, 1}, {0, 0, 1}, {0.5f, 0.5f}}, // top middle
        };

        std::vector<MeshBuffer::IndexType> indices{
            0, 1, 2
        };

        return CreateMeshBuilder().SetVertexData(vertices).SetIndices(indices).Create();
    }
}
