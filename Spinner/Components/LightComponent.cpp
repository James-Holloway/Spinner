#include "LightComponent.hpp"

#include "../SceneObject.hpp"
#include "../Constants.hpp"
#include "../Scene.hpp"
#include "../DrawCommand.hpp"
#include "MeshComponent.hpp"

#include <imgui.h>

namespace Spinner
{
    namespace Components
    {
        LightComponent::LightComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<LightComponent>(), componentIndex)
        {
            SetupShadows({ShadowMapWidth, ShadowMapWidth});
        }

        Spinner::LightType LightComponent::GetLightType() const
        {
            return LightType;
        }

        void LightComponent::SetLightType(Spinner::LightType lightType)
        {
            LightType = lightType;
            SetupShadows({ShadowMapWidth, ShadowMapWidth});
        }

        glm::vec3 LightComponent::GetLightColor() const
        {
            return LightColor;
        }

        void LightComponent::SetLightColor(glm::vec3 lightColor)
        {
            LightColor = lightColor;
        }

        float LightComponent::GetLightStrength() const
        {
            return LightStrength;
        }

        void LightComponent::SetLightStrength(float strength)
        {
            LightStrength = strength;
        }

        float LightComponent::GetInnerSpotAngle() const
        {
            return InnerSpotAngle;
        }

        void LightComponent::SetInnerSpotAngle(float angle)
        {
            InnerSpotAngle = angle;
        }

        float LightComponent::GetOuterSpotAngle() const
        {
            return OuterSpotAngle;
        }

        void LightComponent::SetOuterSpotAngle(float angle)
        {
            OuterSpotAngle = angle;
        }

        bool LightComponent::GetIsShadowCaster() const
        {
            return IsShadowCaster;
        }

        void LightComponent::SetIsShadowCaster(bool isShadowCaster)
        {
            IsShadowCaster = isShadowCaster;
        }

        Spinner::Light LightComponent::GetLight() const
        {
            const auto sceneObject = SceneObject.lock();
            assert(sceneObject != nullptr);

            Spinner::Light light;

            light.SetPosition(sceneObject->GetWorldPosition());
            light.SetDirection(sceneObject->GetWorldRotation() * AxisForward);
            light.SetLightType(LightType);
            light.SetColor(LightColor * LightStrength);
            light.SetIsShadowCaster(IsShadowCaster);

            if (LightType == Spinner::LightType::Spot)
            {
                light.SetInnerSpotAngle(InnerSpotAngle);
                light.SetOuterSpotAngle(OuterSpotAngle);
            }

            // Shadow Matrix
            const auto view = GetShadowViewMatrix();
            const auto projection = GetShadowProjectionMatrix();
            light.SetShadowMatrix(projection * view);

            return light;
        }

        Spinner::Image::Pointer LightComponent::GetShadowMapImage() const
        {
            return ShadowMapImage;
        }

        vk::ImageView LightComponent::GetShadowMapImageDepthView() const
        {
            return ShadowMapImageView;
        }

        glm::mat4 LightComponent::GetShadowProjectionMatrix() const
        {
            switch (LightType)
            {
                default:
                case LightType::None:
                    return glm::mat4(1.0f);
                case LightType::Point:
                    return glm::perspectiveLH(glm::radians(90.0f), 1.0f, 0.01f, 50.0f);
                case LightType::Spot:
                    return glm::perspectiveLH(OuterSpotAngle * 2.0f, 1.0f, 0.01f, 140.0f);
                case LightType::Directional:
                {
                    // TODO get scene bounds for directional sun rendering
                    constexpr float radius = 35.0f;

                    return glm::ortho(-radius, radius, -radius, radius, 1.0f, radius * 4.0f);
                }
            }
        }

        glm::mat4 LightComponent::GetShadowViewMatrix() const
        {
            auto lightSceneObject = SceneObject.lock();
            if (lightSceneObject == nullptr)
            {
                return glm::mat4{1.0f};
            }

            const auto world = lightSceneObject->GetWorldMatrix();
            auto view = glm::inverse(world);

            switch (LightType)
            {
                case LightType::Directional:
                {
                    const auto worldPos = lightSceneObject->GetWorldPosition();
                    view = glm::lookAt((-glm::vec3(view[2]) * 100.0f) + worldPos, worldPos, glm::vec3(view[1]));
                    break;
                }
                case LightType::Spot:
                {
                    // Rotate around 180 deg yaw to fix direction being backwards
                    const auto rotatedWorld = world * glm::rotate(glm::mat4{1.0f}, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    view = glm::inverse(rotatedWorld);
                    break;
                }
                default:
                    break;
            }

            return view;
        }

        static std::shared_ptr<DrawCommand> CreateShadowDrawCommand(const Spinner::DescriptorPool::Pointer &descriptorPool, const Spinner::Components::MeshComponent *meshComponent)
        {
            return std::make_shared<Spinner::DrawCommand>(meshComponent->GetShadowShaderGroup(), descriptorPool);
        }

        void LightComponent::RenderShadowFace(uint32_t faceIndex, CommandBuffer::Pointer &commandBuffer, const Spinner::DescriptorPool::Pointer &descriptorPool)
        {
            // TODO face rendering following something like https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmappingomni/shadowmappingomni.cpp#L394
        }

        void LightComponent::RenderShadow(CommandBuffer::Pointer &commandBuffer, const Spinner::DescriptorPool::Pointer &descriptorPool)
        {
            auto lightSceneObject = GetSceneObject();
            if (lightSceneObject == nullptr)
            {
                return;
            }

            if (!IsShadowCaster)
            {
                return;
            }

            auto scene = lightSceneObject->GetSceneParent();
            if (scene == nullptr)
            {
                return;
            }

            switch (LightType)
            {
                case LightType::None:
                    return;
                case LightType::Point:
                {
                    for (uint32_t fi = 0; fi < 6; fi++)
                    {
                        RenderShadowFace(fi, commandBuffer, descriptorPool);
                    }
                    return;
                }
                case LightType::Spot:
                case LightType::Directional:
                    break;
                default:
                    throw std::runtime_error("Unhandled light component light type for LightComponent's RenderShadow");
            }

            auto &shadowMapImage = ShadowMapImage;
            auto &shadowMapImageDepthView = ShadowMapImageView;

            if (shadowMapImage == nullptr || shadowMapImageDepthView == nullptr)
            {
                return;
            }

            auto extent = shadowMapImage->GetExtent2D();
            glm::vec2 cameraExtent = {static_cast<float>(extent.width), static_cast<float>(extent.height)};

            const auto projection = GetShadowProjectionMatrix();
            const auto view = GetShadowViewMatrix();

            auto position = lightSceneObject->GetWorldPosition();

            vk::RenderingAttachmentInfo depthAttachmentInfo;
            depthAttachmentInfo.imageView = shadowMapImageDepthView;
            depthAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
            depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            depthAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0u}; // Depth clear value

            vk::Rect2D renderArea({0, 0}, shadowMapImage->GetExtent2D());

            vk::RenderingInfo renderingInfo;
            renderingInfo.renderArea = renderArea;
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = 0;
            renderingInfo.pDepthAttachment = &depthAttachmentInfo;

            commandBuffer->BeginRendering(renderingInfo, shadowMapImage->GetExtent2D());
            commandBuffer->SetDrawParameters(vk::CullModeFlagBits::eFront);

            SceneConstants sceneConstants{};

            const auto &shadowSceneBuffer = ShadowSceneBuffers.at(0);

            sceneConstants.CameraExtent = cameraExtent;
            sceneConstants.CameraPosition = position;
            sceneConstants.View = view;
            sceneConstants.Projection = projection;
            sceneConstants.ViewProjection = projection * view;

            shadowSceneBuffer->Write<SceneConstants>(sceneConstants);
            commandBuffer->TrackObject(shadowSceneBuffer);

            scene->GetObjectTree()->TraverseActive([&](const Spinner::SceneObject::Pointer &meshSceneObject) -> bool
            {
                auto meshComponents = meshSceneObject->GetComponentRawPointers<Components::MeshComponent>();
                for (auto &meshComponent : meshComponents)
                {
                    if (!meshComponent->GetActive())
                        continue;

                    // Update constant buffer with position
                    auto meshConstants = meshComponent->GetMeshConstants();
                    meshConstants.Model = meshSceneObject->GetWorldMatrix();
                    meshComponent->UpdateConstantBuffer(meshConstants);

                    // Create main draw command
                    auto drawCommand = CreateShadowDrawCommand(descriptorPool, meshComponent);
                    drawCommand->UseSceneBuffer(shadowSceneBuffer);

                    meshComponent->UpdateShadow(drawCommand);

                    // Render
                    drawCommand->DrawMesh(commandBuffer);
                }

                return true;
            }, nullptr);

            commandBuffer->EndRendering();
        }

        void LightComponent::SetupShadows(vk::Extent2D shadowTextureSize)
        {
            // Destroy any previous images
            /*if (ShadowMapImage != nullptr)
            {
                ShadowMapImage->DestroyImageView(ShadowMapImageView);
                ShadowMapImageView = nullptr;

                for (auto &cubeImageView : ShadowCubeMapImageView)
                {
                    ShadowMapImage->DestroyImageView(cubeImageView);
                    cubeImageView = nullptr;
                }
            }*/

            // Create new images
            if (LightType == LightType::Point)
            {
                ShadowMapImage = Image::CreateCubeImage(shadowTextureSize, ShadowMapFormat, ShadowMapUsage);
                ShadowSceneBuffers.resize(6);
                ShadowMapImageView = ShadowMapImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::eCube, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 6});
                for (uint32_t i = 0; i < 6; i++)
                {
                    ShadowCubeMapImageView[i] = ShadowMapImage->CreateImageView(vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::e2D, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, i, 1});
                    ShadowSceneBuffers[i] = Buffer::CreateBuffer(sizeof(SceneConstants), vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu, 0, true);
                }
            }
            else
            {
                ShadowMapImage = Image::CreateImage(shadowTextureSize, ShadowMapFormat, ShadowMapUsage);
                ShadowMapImageView = ShadowMapImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth);
                ShadowSceneBuffers = {Buffer::CreateBuffer(sizeof(SceneConstants), vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu, 0, true)};
            }
        }

        void LightComponent::RenderDebugUI()
        {
            BaseRenderDebugUI();

            static const char *lightTypes[] = {"None", "Point", "Spot", "Directional"};

            auto lightType = GetLightType();
            auto lightTypeInt = static_cast<int>(lightType);
            if (ImGui::Combo("Light Type", &lightTypeInt, lightTypes, 4))
            {
                SetLightType(static_cast<Spinner::LightType>(lightTypeInt));
            }

            glm::vec3 lightColor = GetLightColor();
            if (ImGui::ColorEdit3("Light Color", &lightColor.x, ImGuiColorEditFlags_Float))
            {
                SetLightColor(lightColor);
            }

            float strength = GetLightStrength();
            if (ImGui::DragFloat("Light Strength", &strength, 10.0f, 0.0f, 100000.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
            {
                SetLightStrength(strength);
            }

            if (ImGui::Checkbox("Shadow Caster", &IsShadowCaster))
            {
            }

            if (lightType == LightType::Spot)
            {
                float innerSpotAngle = glm::degrees(GetInnerSpotAngle());
                if (ImGui::DragFloat("Inner Spot Angle", &innerSpotAngle, 0.05f, 0.0f, 179.9f))
                {
                    SetInnerSpotAngle(glm::radians(innerSpotAngle));
                }

                float outerSpotAngle = glm::degrees(GetOuterSpotAngle());
                if (ImGui::DragFloat("Outer Spot Angle", &outerSpotAngle, 0.05f, 0.0f, 179.9f))
                {
                    SetOuterSpotAngle(glm::radians(outerSpotAngle));
                }
            }
        }
    } // Components
} // Spinner
