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
        DescriptorPool = Spinner::DescriptorPool::CreateDefault(4000);
    }

    DrawManager::~DrawManager()
    {
        SceneBuffer.reset();
    }

    Spinner::DrawCommand::Pointer DrawManager::CreateDrawCommand(const Spinner::ShaderGroup::Pointer &shaderGroup)
    {
        return std::make_shared<DrawCommand>(shaderGroup, DescriptorPool);
    }

    void DrawManager::SetScene(const std::shared_ptr<Spinner::Scene> &scene)
    {
        Scene = scene;
    }

    void DrawManager::Update(const Components::ComponentPtr<Components::CameraComponent> &cameraComponent)
    {
        DescriptorPool->ResetPool();
        DrawCommands.clear();

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

        std::vector<Components::LightComponent *> activeLightComponents;

        const auto lighting = scene->GetLighting();


        // Get active light components
        scene->GetObjectTree()->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto lightComponents = sceneObject->GetComponentRawPointers<Components::LightComponent>();
            for (auto &lightComponent : lightComponents)
            {
                if (lightComponent->GetActive())
                {
                    activeLightComponents.push_back(lightComponent);
                }
            }

            return true;
        }, nullptr);

        if (lighting != nullptr)
        {
            const glm::vec3 viewerPosition = LocalSceneBuffer.CameraPosition;

            lighting->UpdateLights(viewerPosition, activeLightComponents);

            // TODO render using a new DrawCommand, the mesh component's ShadowShaderGroup, and the light component's shadow texture
        }

        // Create draw commands
        auto currentFrame = Graphics::GetCurrentFrame();
        scene->GetObjectTree()->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto meshComponents = sceneObject->GetComponentRawPointers<Components::MeshComponent>();
            for (auto &meshComponent : meshComponents)
            {
                if (!meshComponent->GetActive())
                    continue;

                // Update constant buffer with position
                auto meshConstants = meshComponent->GetMeshConstants();
                meshConstants.Model = sceneObject->GetWorldMatrix();
                meshComponent->UpdateConstantBuffer(meshConstants);

                // Create main draw command
                auto drawCommand = CreateDrawCommand(meshComponent->GetShaderGroup());
                drawCommand->UseSceneBuffer(SceneBuffer);
                drawCommand->UseLighting(lighting);

                meshComponent->Update(drawCommand);

                DrawCommands.emplace(drawCommand->GetPass(), drawCommand);
            }

            return true;
        });
    }

    void DrawManager::Render(CommandBuffer::Pointer &commandBuffer)
    {
        auto scene = Scene.lock();
        if (scene == nullptr || !scene->IsActive())
        {
            return;
        }

        // Iterates over in a non-descending order (a lower pass index goes before a higher pass index)
        for (const auto &[set, drawCommand] : DrawCommands)
        {
            drawCommand->DrawMesh(commandBuffer);
        }
    }

    void DrawManager::RenderShadows(CommandBuffer::Pointer &commandBuffer) const
    {
        auto scene = Scene.lock();
        if (scene == nullptr || !scene->IsActive())
        {
            return;
        }

        const auto lighting = scene->GetLighting();
        if (lighting == nullptr)
        {
            return;
        }

        // Track the tracked shadow images
        for (const auto &si : lighting->ShadowImages)
        {
            commandBuffer->TrackObject(si);
        }

        const auto currentFrame = Graphics::GetCurrentFrame();

        commandBuffer->TrackObject(lighting->ShadowSampler);
        commandBuffer->TrackObject(lighting->LightBuffers[currentFrame]);
        commandBuffer->TrackObject(lighting->LightInfoBuffers[currentFrame]);

        for (auto &lightComponent : lighting->SortedLightComponents)
        {
            lightComponent->RenderShadow(commandBuffer, DescriptorPool);
        }
    }
}
