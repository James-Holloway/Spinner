#include "DrawManager.hpp"

#include "Components/Components.hpp"
#include "SceneDescriptors.hpp"
#include "UpdateDescriptors.hpp"
#include "Scene.hpp"
#include "Graphics.hpp"
#include "Lighting.hpp"

namespace Spinner
{
    DrawManager::DrawManager()
    {
        DescriptorPool = DescriptorPool::CreateDefault();
        SceneDescriptorSetLayout = SceneDescriptors::GetDescriptorSetLayout();

        SceneConstantsBuffer = Buffer::CreateBuffer(sizeof(SceneConstants), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eCpuToGpu, 0, true);
    }

    void DrawManager::DrawPass(CommandBuffer::Pointer &commandBuffer, Components::CameraComponent *cameraComponent, const std::map<std::string, std::vector<Components::MeshComponent *>> &meshComponents, bool transparency)
    {
        for (auto &pair : meshComponents)
        {
            bool isFirst = true;
            for (auto &meshComponent : pair.second)
            {
                if (isFirst) [[unlikely]]
                {
                    auto fragShader = meshComponent->GetFragmentShader();
                    meshComponent->BindShaders(commandBuffer);
                    commandBuffer->BindDescriptors(fragShader->GetPipelineLayout(), fragShader->GetSceneDescriptorSetIndex(), SceneDescriptorSets.at(0));
                    isFirst = false;
                }

                meshComponent->BindShaders(commandBuffer);
                meshComponent->Draw(commandBuffer);
            }
        }
    }

    void DrawManager::Draw(CommandBuffer::Pointer &commandBuffer, Components::CameraComponent *cameraComponent, const std::map<std::string, std::vector<Components::MeshComponent *>> &meshComponents)
    {
        assert(cameraComponent != nullptr);

        // Check render target on the camera exists
        auto renderTarget = cameraComponent->GetRenderTarget();
        if (renderTarget == nullptr)
        {
            throw std::runtime_error("Cannot render to a nullptr RenderTarget from a cameraComponent in DrawManager::Draw");
        }

        // Check that either color or depth textures exist
        auto colorTexture = renderTarget->GetColorTexture();
        auto depthTexture = renderTarget->GetDepthTexture();

        if (colorTexture == nullptr && depthTexture == nullptr)
        {
            throw std::runtime_error("Cannot render to a RenderTarget with no Color or Depth textures set");
        }

        vk::ImageView colorImageView{nullptr}, depthImageView{nullptr};
        vk::Extent2D drawExtent = {0, 0};

        // Get draw extent and image views from the textures
        if (depthTexture != nullptr && depthTexture->GetImage() != nullptr)
        {
            depthImageView = depthTexture->GetMainImageView();
            drawExtent = depthTexture->GetImage()->GetExtent2D();
        }
        if (colorTexture != nullptr && colorTexture->GetImage() != nullptr)
        {
            colorImageView = colorTexture->GetMainImageView();
            drawExtent = colorTexture->GetImage()->GetExtent2D();
        }

        // Check draw extent
        if (drawExtent == vk::Extent2D{0, 0})
        {
            throw std::runtime_error("Cannot render to a RenderTarget with a draw extent of 0,0");
        }

        renderTarget->TransitionForRendering(commandBuffer);

        commandBuffer->BeginRendering(colorImageView, depthImageView, {{0, 0}, drawExtent});
        {
            // Opaque pass
            DrawPass(commandBuffer, cameraComponent, meshComponents, false);
            // Transparent pass
            // DrawPass(commandBuffer, cameraComponent, meshComponents, true);
        }
        commandBuffer->EndRendering();

        renderTarget->TransitionForRead(commandBuffer);
    }

    void DrawManager::ResetAndUpdate(Components::CameraComponent *cameraComponent, const std::shared_ptr<Scene> &scene)
    {
        assert(cameraComponent != nullptr);
        assert(scene != nullptr);

        SceneDescriptorSets.clear();
        DescriptorPool->ResetPool();

        SceneDescriptorSets = DescriptorPool->AllocateDescriptorSets(SceneDescriptorSetLayout->GetDescriptorSetLayout());

        // Update scene constants from camera component (view + projection)
        SceneConstants sceneConstants{};
        cameraComponent->UpdateSceneConstants(sceneConstants);
        SceneConstantsBuffer->Write(sceneConstants, nullptr);

        for (auto &set : SceneDescriptorSets)
        {
            UpdateDescriptors::UpdateDescriptorBuffer(set, 0, vk::DescriptorType::eUniformBuffer, SceneConstantsBuffer);
        }

        std::vector<Components::CameraComponent *> cameraComponents;

        // TODO shader time + delta time

        // Update scene constants
        glm::vec3 viewerPosition = sceneConstants.CameraPosition;

        std::vector<Components::LightComponent *> activeLightComponents;

        MeshComponentShaderMap.clear();
        DrawingCameraComponent = cameraComponent;

        // Update mesh constants and grab all meshes
        auto lighting = scene->GetLighting();
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
                meshComponent->Update(lighting, currentFrame);

                auto meshShaderName = meshComponent->GetFragmentShader()->GetShaderName();
                MeshComponentShaderMap[meshShaderName].push_back(meshComponent);
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

        lighting->UpdateLights(viewerPosition, activeLightComponents);
    }

    DrawManager::Pointer DrawManager::Create()
    {
        return std::make_shared<Spinner::DrawManager>();
    }

    void DrawManager::Draw(CommandBuffer::Pointer &commandBuffer)
    {
        Draw(commandBuffer, DrawingCameraComponent, MeshComponentShaderMap);
    }
} // Spinner