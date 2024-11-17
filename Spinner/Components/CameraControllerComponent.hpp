#ifndef SPINNER_CAMERACONTROLLER_HPP
#define SPINNER_CAMERACONTROLLER_HPP

#include "Component.hpp"
#include "CameraComponent.hpp"

namespace Spinner
{
    class CommandBuffer;

    namespace Components
    {
        // Right click & WASDEQ movement when this component and the active camera share the same SceneObject
        class CameraControllerComponent : public Component
        {
        public:
            CameraControllerComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            float MouseSensitivity = 1.0f;
            int BaseMovementSpeed = 0;

            constexpr static float BaseMovementSpeedModifier = 1.0f / 16.0f;
            constexpr static int MaxBaseMovementSpeed = 8;

        public:
            void Update(float deltaTime);
            void RenderDebugUI();
        };

        template<>
        inline constexpr ComponentId GetComponentId<CameraControllerComponent>()
        {
            return 5;
        }

        template<>
        inline const char *GetComponentName<CameraControllerComponent>()
        {
            return "CameraControllerComponent";
        }

        template<>
        inline void RenderDebugUI<CameraControllerComponent>(CameraControllerComponent *component)
        {
            component->RenderDebugUI();
        }
    }
}

#endif
