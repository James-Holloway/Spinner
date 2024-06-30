#include "LightComponent.hpp"

#include "../SceneObject.hpp"

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
    } // Components
} // Spinner