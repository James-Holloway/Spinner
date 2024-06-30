#include "Light.hpp"

#include "cassert"

namespace Spinner
{
    constexpr const static uint32_t LightFlags_TypeMask = 0b111;
    constexpr const static uint32_t LightFlags_ShadowCaster = 0b1000;

    LightType Light::GetLightType() const
    {
        return static_cast<LightType>(Flags & LightFlags_TypeMask);
    }

    void Light::SetLightType(LightType lightType)
    {
        assert(static_cast<uint32_t>(lightType) <= LightFlags_TypeMask);

        Flags &= ~LightFlags_TypeMask;
        Flags |= (static_cast<uint32_t>(lightType) & LightFlags_TypeMask);
    }

    bool Light::GetIsShadowCaster() const
    {
        return (Flags & LightFlags_ShadowCaster) == LightFlags_ShadowCaster;
    }

    void Light::SetIsShadowCaster(bool isShadowCaster)
    {
        if (isShadowCaster)
        {
            Flags |= LightFlags_ShadowCaster;
        }
        else
        {
            Flags &= ~LightFlags_ShadowCaster;
        }
    }

    glm::vec3 Light::GetColor() const
    {
        return {Red, Green, Blue};
    }

    void Light::SetColor(glm::vec3 color)
    {
        Red = color.x;
        Green = color.y;
        Blue = color.z;
    }

    glm::vec3 Light::GetPosition() const
    {
        return {Position};
    }

    void Light::SetPosition(glm::vec3 position)
    {
        Position = glm::vec4(position, Position.w);
    }

    glm::vec3 Light::GetDirection() const
    {
        return {Direction};
    }

    void Light::SetDirection(glm::vec3 direction)
    {
        Direction = glm::vec4(direction, Direction.w);
    }

    float Light::GetInnerSpotAngle() const
    {
        return ExtraData.x;
    }

    void Light::SetInnerSpotAngle(float angle)
    {
        ExtraData.x = angle;
    }

    float Light::GetOuterSpotAngle() const
    {
        return ExtraData.y;
    }

    void Light::SetOuterSpotAngle(float angle)
    {
        ExtraData.y = angle;
    }

    glm::mat4 Light::GetShadowMatrix() const
    {
        return ShadowMatrix;
    }

    void Light::SetShadowMatrix(glm::mat4 shadowMatrix)
    {
        ShadowMatrix = shadowMatrix;
    }
} // Spinner