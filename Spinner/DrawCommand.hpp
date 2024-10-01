#ifndef SPINNER_DRAWCOMMAND_HPP
#define SPINNER_DRAWCOMMAND_HPP

#include "DescriptorPool.hpp"
#include "Lighting.hpp"
#include "Material.hpp"
#include "MeshBuffer.hpp"
#include "Passes.hpp"
#include "Shader.hpp"

namespace Spinner
{
    class DrawCommand final
    {
    public:
        using Pointer = std::shared_ptr<DrawCommand>;

        DrawCommand(Spinner::ShaderGroup::Pointer shaderGroup, const Spinner::DescriptorPool::Pointer &descriptorPool);
        ~DrawCommand() = default;

    protected:
        std::vector<vk::DescriptorSet> DescriptorSets;
        Spinner::ShaderGroup::Pointer ShaderGroup;
        Spinner::Shader::Pointer OperatingShader; // Typically the fragment shader, the update functions will operate with this shader

    protected:
        Spinner::MeshBuffer::Pointer MeshBuffer;
        Spinner::Material::Pointer Material;
        Spinner::Lighting::Pointer Lighting;
        Spinner::Buffer::Pointer SceneBuffer;

        Spinner::Pass Pass = OpaquePass;

    public:
        void UseMeshBuffer(const Spinner::MeshBuffer::Pointer &meshBuffer);
        void UseMaterial(const Spinner::Material::Pointer &material);
        void UseLighting(const Spinner::Lighting::Pointer &lighting);
        void UseSceneBuffer(const Spinner::Buffer::Pointer &sceneBuffer);

        [[nodiscard]] Spinner::Pass GetPass() const;
        void UsePass(Spinner::Pass pass);

        void DrawMesh(const CommandBuffer::Pointer &commandBuffer);

        [[nodiscard]] uint32_t GetDescriptorSetCount() const;
        [[nodiscard]] vk::DescriptorSet GetDescriptorSet(uint32_t set) const;

        [[nodiscard]] Spinner::Shader::Pointer GetShader(vk::ShaderStageFlagBits shaderStage) const;

        void UpdateDescriptorBuffer(uint32_t binding, const std::shared_ptr<Buffer> &buffer, uint32_t set = 0) const;
        void UpdateDescriptorImage(uint32_t binding, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal, uint32_t set = 0) const;
        void UpdateDescriptorImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal, uint32_t set = 0) const;
    };
}

#endif
