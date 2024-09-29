#include "DrawManager.hpp"

#include "Graphics.hpp"
#include "Lighting.hpp"
#include "Scene.hpp"
#include "Components/LightComponent.hpp"

namespace Spinner
{
    DrawManager::DrawManager()
    {
        SceneBuffer = Buffer::CreateBuffer(sizeof(SceneConstants), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eCpuToGpu, 0, true);
    }

    DrawManager::~DrawManager()
    {
        SceneBuffer.reset();
    }

    void DrawManager::SetScene(const std::shared_ptr<Spinner::Scene> &scene)
    {
        Scene = scene;
    }

    void DrawManager::Update(const Components::ComponentPtr<Components::CameraComponent> &cameraComponent)
    {
        auto scene = Scene.lock();
        if (scene == nullptr || !scene->IsActive())
        {
            return;
        }

        if (!cameraComponent.IsValid())
        {
            return;
        }

        cameraComponent->UpdateSceneConstants(LocalSceneBuffer);
        SceneBuffer->Write<SceneConstants>(LocalSceneBuffer);

        glm::vec3 viewerPosition = LocalSceneBuffer.CameraPosition;

        std::vector<Components::LightComponent *> activeLightComponents;

        // Update mesh constants
        auto currentFrame = Graphics::GetCurrentFrame();
        scene->GetObjectTree()->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto meshComponents = sceneObject->GetComponentRawPointers<Components::MeshComponent>();
            for (auto &meshComponent : meshComponents)
            {
                // Update constant buffer with position
                auto meshConstants = meshComponent->GetMeshConstants();
                meshConstants.Model = sceneObject->GetWorldMatrix();
                meshComponent->UpdateConstantBuffer(meshConstants);

                // Potentially update descriptor sets
                meshComponent->Update(scene, SceneBuffer, currentFrame);
            }

            // Get active light components
            auto lightComponents = sceneObject->GetComponentRawPointers<Components::LightComponent>();
            for (auto &lightComponent : lightComponents)
            {
                if (lightComponent->GetActive())
                {
                    activeLightComponents.push_back(lightComponent);
                }
            }

            return true;
        });

        const auto lighting = scene->GetLighting();
        if (lighting != nullptr)
        {
            lighting->UpdateLights(viewerPosition, activeLightComponents);
        }
    }

    void DrawManager::Render(CommandBuffer::Pointer &commandBuffer)
    {
        auto scene = Scene.lock();
        if (scene == nullptr || !scene->IsActive())
        {
            return;
        }

        std::map<std::string, std::vector<Components::MeshComponent *>> toDrawOpaque;
        scene->GetObjectTree()->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto meshComponents = sceneObject->GetComponentRawPointers<Components::MeshComponent>();
            for (auto &meshComponent : meshComponents)
            {
                auto shaderName = meshComponent->GetFragmentShaderInstance()->GetShader()->GetShaderName();
                toDrawOpaque[shaderName].push_back(meshComponent);
            }

            return true;
        });

        // Opaque pass
        Shader::Pointer prevFragShader = nullptr;
        for (const auto &[shaderName, meshComponents] : toDrawOpaque)
        {
            if (meshComponents.empty())
                continue;

            for (auto &meshComponent : meshComponents)
            {
                meshComponent->Draw(commandBuffer);
            }
        }

        // TODO transparent pass
    }
}
