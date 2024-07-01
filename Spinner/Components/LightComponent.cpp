#include "LightComponent.hpp"

#include "../SceneObject.hpp"
#include <imgui.h>

namespace Spinner
{
    namespace Components
    {
        LightComponent::LightComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex) : Component(sceneObject, Components::GetComponentId<LightComponent>(), componentIndex)
        {

        }

        Spinner::LightType LightComponent::GetLightType() const
        {
            return LightType;
        }

        void LightComponent::SetLightType(Spinner::LightType lightType)
        {
            LightType = lightType;
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
            auto sceneObject = SceneObject.lock();
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

            // TODO Apply shadow matrix

            return light;
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