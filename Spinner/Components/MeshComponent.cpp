#include "MeshComponent.hpp"

#include <utility>

#include "../CommandBuffer.hpp"
#include "../VulkanInstance.hpp"
#include "../Scene.hpp"
#include "../DrawCommand.hpp"
#include "../Lighting.hpp"
#include <imgui.h>

namespace Spinner::Components
{
    MeshComponent::MeshComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<MeshComponent>(), componentIndex)
    {
        ConstantBuffer = Buffer::CreateBuffer(sizeof(ConstantBufferType), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eCpuToGpu, 0, true);
    }

    Spinner::ShaderGroup::Pointer MeshComponent::GetShaderGroup() const
    {
        return ShaderGroup;
    }

    void MeshComponent::SetShaderGroup(const Spinner::ShaderGroup::Pointer &shaderGroup)
    {
        ShaderGroup = shaderGroup;
    }

    Spinner::MeshBuffer::Pointer MeshComponent::GetMeshBuffer()
    {
        return MeshBuffer;
    }

    void MeshComponent::SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshShader)
    {
        MeshBuffer = std::move(newMeshShader);
    }

    Spinner::Material::Pointer MeshComponent::GetMaterial()
    {
        return Material;
    }

    void MeshComponent::SetMaterial(const Material::Pointer &material)
    {
        Material = material;
    }

    void MeshComponent::UpdateConstantBuffer(const MeshComponent::ConstantBufferType &constants)
    {
        LocalConstantBuffer = constants;
        ConstantBuffer->Write<ConstantBufferType>(LocalConstantBuffer);
    }

    MeshComponent::ConstantBufferType MeshComponent::GetMeshConstants() const
    {
        return LocalConstantBuffer;
    }

    Spinner::Buffer::Pointer MeshComponent::GetMeshConstantsBuffer() const
    {
        return ConstantBuffer;
    }

    void MeshComponent::Update(const std::shared_ptr<DrawCommand> &drawCommand)
    {
        // Cannot render without material or shader group
        if (Material == nullptr || ShaderGroup == nullptr)
            return;

        // Update material
        auto constants = GetMeshConstants();
        Material->ApplyMaterial(constants);
        UpdateConstantBuffer(constants);

        drawCommand->UseMeshBuffer(MeshBuffer);
        drawCommand->UseMaterial(Material);

        if (Material->IsTransparent())
        {
            drawCommand->UsePass(TransparentPass);
        }

        // Run shader specific code (e.g. binding mesh constants buffer)
        ShaderGroup->RunUpdateDrawComponentCallbacks(drawCommand, this);
    }

    void MeshComponent::RenderDebugUI()
    {
        BaseRenderDebugUI();

        if (MeshBuffer != nullptr)
        {
            ImGui::Text("Mesh Buffer Index Count: %u", MeshBuffer->IndexCount);
            ImGui::Text("Mesh Buffer Total Buffer Size: %lu", MeshBuffer->BufferSize);
        }
        else
        {
            ImGui::Text("No Mesh Buffer");
        }

        ImGui::Indent(8);
        if (Material != nullptr)
        {
            ImGui::PushID("Material");
            Material->RenderDebugUI();
            ImGui::PopID();
        }
        else
        {
            ImGui::Text("No Material");
        }
        ImGui::Unindent(8);
    }
}
