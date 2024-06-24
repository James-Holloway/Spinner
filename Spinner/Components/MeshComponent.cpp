#include "MeshComponent.hpp"

#include <utility>

#include "../CommandBuffer.hpp"
#include "../VulkanInstance.hpp"

namespace Spinner::Components
{
    Spinner::MeshBuffer::Pointer MeshComponent::GetMeshBuffer()
    {
        return MeshBuffer;
    }

    void MeshComponent::SetVertexShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance)
    {
        VertexShaderInstance = std::move(newShaderInstance);
    }

    void MeshComponent::SetFragmentShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance)
    {
        FragmentShaderInstance = std::move(newShaderInstance);
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
        commandBuffer->DrawMesh(MeshBuffer);
        commandBuffer->TrackObject(shared_from_this());
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
    }
}