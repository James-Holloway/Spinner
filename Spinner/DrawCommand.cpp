#include "DrawCommand.hpp"

#include <utility>

#include "Buffer.hpp"
#include "Graphics.hpp"
#include "Scene.hpp"
#include "Texture.hpp"

namespace Spinner
{
    DrawCommand::DrawCommand(Spinner::ShaderGroup::Pointer shaderGroup, const Spinner::DescriptorPool::Pointer &descriptorPool) : ShaderGroup(std::move(shaderGroup))
    {
        if (ShaderGroup == nullptr)
        {
            throw std::invalid_argument("shaderGroup is null");
        }
        if (descriptorPool == nullptr)
        {
            throw std::invalid_argument("descriptorPool is null");
        }

        if (ShaderGroup->HasShaderStage(vk::ShaderStageFlagBits::eFragment))
        {
            OperatingShader = ShaderGroup->GetShader(vk::ShaderStageFlagBits::eFragment);

            DescriptorSets = descriptorPool->AllocateDescriptorSets(OperatingShader);
        }
        else
        {
            throw std::runtime_error("Cannot allocate descriptor sets from a ShaderGroup without a fragment stage");
        }
    }

    void DrawCommand::UseMeshBuffer(const Spinner::MeshBuffer::Pointer &meshBuffer)
    {
        MeshBuffer = meshBuffer;
    }

    void DrawCommand::UseMaterial(const Spinner::Material::Pointer &material)
    {
        Material = material;
        Material->ApplyTextures(this);
    }

    void DrawCommand::UseLighting(const Spinner::Lighting::Pointer &lighting)
    {
        Lighting = lighting;

        if (Lighting != nullptr)
        {
            const auto lightingSetIndex = OperatingShader->GetLightingDescriptorSetIndex();
            if (lightingSetIndex != Shader::InvalidBindingIndex)
            {
                Lighting->UpdateDescriptors(DescriptorSets.at(lightingSetIndex));
            }
        }
    }

    void DrawCommand::UseSceneBuffer(const Spinner::Buffer::Pointer &sceneBuffer)
    {
        SceneBuffer = sceneBuffer;

        if (SceneBuffer != nullptr)
        {
            const auto sceneSetIndex = OperatingShader->GetSceneDescriptorSetIndex();
            if (sceneSetIndex != Shader::InvalidBindingIndex)
            {
                UpdateDescriptorBuffer(Scene::SceneUniformBufferBindingIndex, SceneBuffer, sceneSetIndex);
            }
        }
    }

    Spinner::Pass DrawCommand::GetPass() const
    {
        return Pass;
    }

    void DrawCommand::UsePass(const Spinner::Pass pass)
    {
        Pass = pass;
    }

    void DrawCommand::DrawMesh(const CommandBuffer::Pointer &commandBuffer)
    {
        if (MeshBuffer == nullptr || Material == nullptr)
        {
            return;
        }

        commandBuffer->TrackObject(ShaderGroup);
        commandBuffer->TrackObject(MeshBuffer);
        commandBuffer->TrackObject(Material);
        commandBuffer->TrackObject(Lighting);
        commandBuffer->TrackObject(SceneBuffer);

        ShaderGroup->BindShaders(commandBuffer);
        commandBuffer->BindDescriptors(OperatingShader->GetPipelineLayout(), 0, DescriptorSets, vk::PipelineBindPoint::eGraphics);
        commandBuffer->DrawMesh(MeshBuffer);
    }

    uint32_t DrawCommand::GetDescriptorSetCount() const
    {
        return static_cast<uint32_t>(DescriptorSets.size());
    }

    vk::DescriptorSet DrawCommand::GetDescriptorSet(const uint32_t set) const
    {
        return DescriptorSets.at(set);
    }

    Spinner::Shader::Pointer DrawCommand::GetShader(const vk::ShaderStageFlagBits shaderStage) const
    {
        return ShaderGroup->GetShader(shaderStage);
    }

    void DrawCommand::UpdateDescriptorBuffer(const uint32_t binding, const std::shared_ptr<Buffer> &buffer, const uint32_t set) const
    {
        const auto descriptorType = OperatingShader->GetDescriptorTypeOfBinding(binding);
        if (!descriptorType.has_value())
        {
            throw std::runtime_error("Cannot get binding type from Shader's descriptor set layout bindings, binding #" + std::to_string(binding));
        }

        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = buffer->VkBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = vk::WholeSize;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = DescriptorSets.at(set);
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType.value();
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    void DrawCommand::UpdateDescriptorImage(uint32_t binding, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout, uint32_t set) const
    {
        assert(texture != nullptr);
        assert(texture->GetImage() != nullptr);
        assert(texture->GetImage()->GetMainImageView() != nullptr);
        assert(texture->GetSampler() != nullptr);

        UpdateDescriptorImage(binding, texture->GetImage()->GetMainImageView(), texture->GetSampler()->GetSampler(), imageLayout, set);
    }

    void DrawCommand::UpdateDescriptorImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout, uint32_t set) const
    {
        auto descriptorType = OperatingShader->GetDescriptorTypeOfBinding(binding);
        if (!descriptorType.has_value())
        {
            throw std::runtime_error("Cannot get binding type from Shader's descriptor set layout bindings, binding #" + std::to_string(binding));
        }

        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = imageLayout;
        imageInfo.sampler = sampler;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = DescriptorSets.at(set);
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType.value();
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}
