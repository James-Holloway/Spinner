#include "CameraComponent.hpp"

#include "../SceneObject.hpp"
#include "../Graphics.hpp"
#include <imgui.h>

namespace Spinner
{
    namespace Components
    {
        std::pair<std::weak_ptr<Spinner::SceneObject>, int64_t> CameraComponent::ActiveCameraComponent;

        CameraComponent::CameraComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<CameraComponent>(), componentIndex)
        {

        }

        bool CameraComponent::IsActiveCamera()
        {
            if (ActiveCameraComponent.first.expired())
            {
                return false;
            }

            auto activeCameraSceneObject = ActiveCameraComponent.first.lock();
            auto activeCameraComponentIndex = ActiveCameraComponent.second;

            return activeCameraSceneObject == SceneObject.lock() && ComponentIndex == activeCameraComponentIndex;
        }

        void CameraComponent::SetActiveCamera()
        {
            ActiveCameraComponent = {SceneObject, ComponentIndex};
        }

        float CameraComponent::GetFOV() const noexcept
        {
            return FOV;
        }

        float CameraComponent::GetNearZ() const noexcept
        {
            return NearZ;
        }

        float CameraComponent::GetFarZ() const noexcept
        {
            return FarZ;
        }

        void CameraComponent::SetFOV(float fov) noexcept
        {
            FOV = fov;
        }

        void CameraComponent::SetNearZ(float nearZ) noexcept
        {
            NearZ = nearZ;
        }

        void CameraComponent::SetFarZ(float farZ) noexcept
        {
            FarZ = farZ;
        }

        void CameraComponent::UpdateSceneConstants(SceneConstants &sceneConstants)
        {
            // TODO use render texture extents if applicable
            auto extent = Graphics::GetSwapchainExtent();
            glm::vec2 cameraExtent = {static_cast<float>(extent.width), static_cast<float>(extent.height)};

            auto sceneObject = SceneObject.lock();
            auto position = sceneObject->GetWorldPosition();
            auto view = glm::inverse(sceneObject->GetWorldMatrix());
            float aspect = cameraExtent.x / cameraExtent.y;
            if (cameraExtent.y > cameraExtent.x)
            {
                aspect = cameraExtent.y / cameraExtent.x;
            }

            sceneConstants.CameraExtent = cameraExtent;
            sceneConstants.CameraPosition = position;
            sceneConstants.View = view;
            sceneConstants.Projection = glm::perspectiveLH(glm::radians(FOV), aspect, NearZ, FarZ);
            sceneConstants.Projection[0][0] *= -1; // invert X direction
            sceneConstants.Projection[1][1] *= -1; // invert Y direction
            sceneConstants.ViewProjection = sceneConstants.Projection * view;
        }

        CameraComponent *CameraComponent::GetActiveCameraRawPointer()
        {
            if (ActiveCameraComponent.first.expired())
            {
                return nullptr;
            }

            return ActiveCameraComponent.first.lock()->GetComponentRawPointer<CameraComponent>(ActiveCameraComponent.second);
        }

        void CameraComponent::RenderDebugUI()
        {
            BaseRenderDebugUI();
            
            float fov = GetFOV();
            if (ImGui::DragFloat("FOV", &fov, 0.1f, 0.5f, 175.0f))
            {
                SetFOV(fov);
            }

            float nearZ = GetNearZ();
            if (ImGui::DragFloat("Near Z", &nearZ, 0.1f, 0.001f, 100.0f, "%.3f"))
            {
                SetNearZ(nearZ);
            }

            float farZ = GetFarZ();
            if (ImGui::DragFloat("Far Z", &farZ, 0.1f, 0.01f, 10000.0f, "%.2f"))
            {
                SetFarZ(farZ);
            }
        }
    } // Components
} // Spinner