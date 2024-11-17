#include "CameraControllerComponent.hpp"

#include "../Graphics.hpp"
#include "../ImGuiInstance.hpp"
#include "../SceneObject.hpp"

namespace Spinner::Components
{
    CameraControllerComponent::CameraControllerComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<CameraControllerComponent>(), componentIndex)
    {
    }

    void CameraControllerComponent::Update(float deltaTime)
    {
        const auto sceneObject = SceneObject.lock();
        if (sceneObject == nullptr)
            return;

        const auto input = Graphics::GetInput();

        if (!input->GetKeyDown(Key::MouseRight))
            return;

        const auto activeCamera = Components::CameraComponent::GetActiveCameraRawPointer();
        if (activeCamera == nullptr)
            return;
\
        // Only allow this controller to control the active camera if it is on the same SceneObject
        if (activeCamera->GetSceneObject() != sceneObject)
        {
            return;
        }

        glm::vec3 movementDirection{0.0f, 0.0f, 0.0f};
        // Forward / Back
        if (input->GetKeyDown(Key::W))
        {
            movementDirection.z += 1.0f;
        }
        if (input->GetKeyDown(Key::S))
        {
            movementDirection.z -= 1.0f;
        }
        // Right / Left
        if (input->GetKeyDown(Key::A))
        {
            movementDirection.x += 1.0f;
        }
        if (input->GetKeyDown(Key::D))
        {
            movementDirection.x -= 1.0f;
        }
        // Up / Down
        if (input->GetKeyDown(Key::E))
        {
            movementDirection.y += 1.0f;
        }
        if (input->GetKeyDown(Key::Q))
        {
            movementDirection.y -= 1.0f;
        }

        float movementSpeed = deltaTime * 4.0f;

        if (BaseMovementSpeed < 0)
        {
            movementSpeed *= 1.0f - (static_cast<float>(1 << -BaseMovementSpeed) * BaseMovementSpeedModifier);
        }
        else if (BaseMovementSpeed > 0)
        {
            movementSpeed *= 1.0f + (static_cast<float>(1 << BaseMovementSpeed) * BaseMovementSpeedModifier);
        }

        if (input->GetKeyDown(Key::LeftShift))
        {
            movementSpeed *= 4.0f;
        }
        if (input->GetKeyDown(Key::LeftControl))
        {
            movementSpeed /= 4.0f;
        }

        if (movementDirection.x != 0.0f || movementDirection.y != 0.0f || movementDirection.z != 0.0f)
        {
            movementDirection = glm::normalize(movementDirection) * movementSpeed;
            activeCamera->MoveRelative(movementDirection);
        }

        // Euler rotation is in degrees, so 90 is in degrees
        float cameraRotationSpeed = 45.0f * MouseSensitivity * 0.01f;
        double cameraX = 0.0, cameraY = 0.0;
        input->GetCursorDeltaPosition(cameraX, cameraY);

        if (cameraX != 0.0f || cameraY != 0.0f)
        {
            const glm::vec3 cameraRotation{cameraRotationSpeed * cameraY, cameraRotationSpeed * -cameraX, 0.0f};
            activeCamera->RotateEuler(cameraRotation);
        }

        // Scroll in and out to change movement speed
        const auto scrollY = static_cast<float>(input->GetVerticalScrollDelta());
        if (scrollY < 0.0f && BaseMovementSpeed > 0)
        {
            BaseMovementSpeed -= 1;
        }
        else if (scrollY > 0.0f && BaseMovementSpeed < MaxBaseMovementSpeed)
        {
            BaseMovementSpeed += 1;
        }
    }

    void CameraControllerComponent::RenderDebugUI()
    {
        Component::BaseRenderDebugUI();

        ImGui::DragFloat("Mouse Sensitivity", &MouseSensitivity, 0.001f, 0.0f, 1.0f, "%.3f");
        ImGui::SliderInt("Base Movement Speed", &BaseMovementSpeed, 0, MaxBaseMovementSpeed);
    }
}
