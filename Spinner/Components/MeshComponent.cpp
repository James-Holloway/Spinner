#include "MeshComponent.hpp"

#include <utility>

#include "../CommandBuffer.hpp"
#include "../VulkanInstance.hpp"
#include "../Scene.hpp"

namespace Spinner::Components
{
    MeshComponent::MeshComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<MeshComponent>(), componentIndex)
    {
        ConstantBuffer = Buffer::CreateBuffer(sizeof(ConstantBufferType), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eCpuToGpu, 0, true);

        RegisterCallback(SceneObject.lock()->ParentChanged, [this](const auto &sceneObject, const auto &scene) -> void
        {
            ConstantsDirtyCount = MAX_FRAMES_IN_FLIGHT;
        });
    }

    Spinner::MeshBuffer::Pointer MeshComponent::GetMeshBuffer()
    {
        return MeshBuffer;
    }

    void MeshComponent::SetVertexShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance)
    {
        VertexShaderInstance = std::move(newShaderInstance);
        ConstantsDirtyCount = MAX_FRAMES_IN_FLIGHT;
    }

    void MeshComponent::SetFragmentShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance)
    {
        FragmentShaderInstance = std::move(newShaderInstance);
        ConstantsDirtyCount = MAX_FRAMES_IN_FLIGHT;
    }

    void MeshComponent::SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshShader)
    {
        MeshBuffer = std::move(newMeshShader);
    }

    void MeshComponent::Draw(const std::shared_ptr<CommandBuffer> &commandBuffer)
    {
        commandBuffer->UnbindShaderStage(vk::ShaderStageFlagBits::eGeometry);

        commandBuffer->BindShaderInstance(VertexShaderInstance);
        commandBuffer->BindShaderInstance(FragmentShaderInstance);
        commandBuffer->TrackObject(MeshBuffer);
        commandBuffer->TrackObject(ConstantBuffer);
        commandBuffer->DrawMesh(MeshBuffer);
    }

    Spinner::ShaderInstance::Pointer MeshComponent::GetVertexShaderInstance()
    {
        return VertexShaderInstance;
    }

    Spinner::ShaderInstance::Pointer MeshComponent::GetFragmentShaderInstance()
    {
        return FragmentShaderInstance;
    }

    void MeshComponent::PopulateFromShaders(const Spinner::Shader::Pointer &vertexShader, const Spinner::Shader::Pointer &fragmentShader, const Spinner::DescriptorPool::Pointer &descriptorPool)
    {
        VertexShaderInstance = ShaderInstance::CreateInstance(vertexShader, descriptorPool);
        FragmentShaderInstance = ShaderInstance::CreateInstance(fragmentShader, descriptorPool);
        ConstantsDirtyCount = true;
    }

    void MeshComponent::UpdateConstantBuffer(const MeshComponent::ConstantBufferType &constants)
    {
        LocalConstantBuffer = constants;
        ConstantBuffer->Write<ConstantBufferType>(LocalConstantBuffer);
    }

    MeshComponent::ConstantBufferType MeshComponent::GetMeshConstants()
    {
        return LocalConstantBuffer;
    }

    void MeshComponent::Update(const std::shared_ptr<Scene> &scene, uint32_t currentFrame)
    {
        if (ConstantsDirtyCount <= 0)
        {
            return;
        }

        VertexShaderInstance->UpdateDescriptorBuffer(currentFrame, 0, scene->GetSceneBuffer());
        VertexShaderInstance->UpdateDescriptorBuffer(currentFrame, 1, ConstantBuffer);

        FragmentShaderInstance->UpdateDescriptorBuffer(currentFrame, 0, scene->GetSceneBuffer());
        FragmentShaderInstance->UpdateDescriptorBuffer(currentFrame, 1, ConstantBuffer);

        ConstantsDirtyCount--;
    }
}