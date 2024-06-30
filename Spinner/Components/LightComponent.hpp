#ifndef SPINNER_LIGHTCOMPONENT_HPP
#define SPINNER_LIGHTCOMPONENT_HPP

#include "Component.hpp"
#include "../Light.hpp"

namespace Spinner
{
    class Scene;

    namespace Components
    {

        class LightComponent : public Component
        {
        public:
            LightComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            float InnerSpotAngle = glm::radians(35.0f);
            float OuterSpotAngle = glm::radians(45.0f);
            Spinner::LightType LightType = Spinner::LightType::None;
            glm::vec3 LightColor = {1.0f, 1.0f, 1.0f};
            float LightStrength = 1.0f;
            bool IsShadowCaster = true;

        public:
            [[nodiscard]] Spinner::LightType GetLightType() const;
            void SetLightType(Spinner::LightType lightType);

            [[nodiscard]] glm::vec3 GetLightColor() const;
            void SetLightColor(glm::vec3 lightColor);
            [[nodiscard]] float GetLightStrength() const;
            void SetLightStrength(float strength);

            // Both inner and outer spot angles are stored in radians
            [[nodiscard]] float GetInnerSpotAngle() const;
            void SetInnerSpotAngle(float angle);
            [[nodiscard]] float GetOuterSpotAngle() const;
            void SetOuterSpotAngle(float angle);

            [[nodiscard]] bool GetIsShadowCaster() const;
            void SetIsShadowCaster(bool isShadowCaster);

            [[nodiscard]] Spinner::Light GetLight() const;
        };

        template<>
        inline constexpr ComponentId GetComponentId<LightComponent>()
        {
            return 4;
        }

        template<>
        inline const char *GetComponentName<LightComponent>()
        {
            return "LightComponent";
        }

    } // Components
} // Spinner

#endif //SPINNER_LIGHTCOMPONENT_HPP
