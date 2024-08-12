#ifndef SPINNER_CAMERACOMPONENT_HPP
#define SPINNER_CAMERACOMPONENT_HPP

#include "Component.hpp"
#include "../Constants.hpp"

namespace Spinner
{
    class CommandBuffer;

    namespace Components
    {
        class CameraComponent : public Component
        {
        public:
            CameraComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            float FOV = 70.0f;
            float NearZ = 0.1f;
            float FarZ = 500.0f;

        public:
            bool IsActiveCamera();
            void SetActiveCamera();
            [[nodiscard]] float GetFOV() const noexcept;
            [[nodiscard]] float GetNearZ() const noexcept;
            [[nodiscard]] float GetFarZ() const noexcept;
            void SetFOV(float fov) noexcept;
            void SetNearZ(float nearZ) noexcept;
            void SetFarZ(float farZ) noexcept;
            void UpdateSceneConstants(SceneConstants &sceneConstants);

            void RenderDebugUI();

        protected:
            static std::pair<std::weak_ptr<Spinner::SceneObject>, int64_t> ActiveCameraComponent;

        public:
            static CameraComponent *GetActiveCameraRawPointer();
        };

        template<>
        inline constexpr ComponentId GetComponentId<CameraComponent>()
        {
            return 3;
        }

        template<>
        inline const char *GetComponentName<CameraComponent>()
        {
            return "CameraComponent";
        }

        template<>
        inline void RenderDebugUI<CameraComponent>(CameraComponent *component)
        {
            component->RenderDebugUI();
        }

    } // Components
} // Spinner

#endif //SPINNER_CAMERACOMPONENT_HPP
