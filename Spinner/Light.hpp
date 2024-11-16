#ifndef SPINNER_LIGHT_HPP
#define SPINNER_LIGHT_HPP

#include <cstdint>
#include "GLM.hpp"

namespace Spinner
{
    constexpr static size_t LightCount = 32;
    constexpr static size_t ShadowCount = 32;

    enum class LightType : uint32_t
    {
        None = 0,
        Point,
        Spot,
        Directional,
    };

    struct Light
    {
        // First 3 bits are LightType. None indicates invalid light
        // Followed by bitflags of IsShadowCaster(8)
        uint32_t Flags = 0u;
        // Color, strength applied in light component. Not vec3 due to alignment issues
        float Red = 1.0f;
        float Green = 1.0f;
        float Blue = 1.0f;
        // Used by Point, Spot
        glm::vec4 Position{0, 0, 0, 0};
        // Used by Spot, Directional
        glm::vec4 Direction{0, -1, 0, 0};
        // Extra data
        // Spot - X InnerSpot Angle, Y OuterSpotAngle
        glm::vec4 ExtraData{0, 0, 0, 0};
        // Shadow Matrix, unused by Point
        glm::mat4 ShadowMatrix{1.0f};

        [[nodiscard]] LightType GetLightType() const;
        void SetLightType(LightType lightType);

        [[nodiscard]] bool GetIsShadowCaster() const;
        void SetIsShadowCaster(bool isShadowCaster);

        [[nodiscard]] glm::vec3 GetColor() const;
        void SetColor(glm::vec3 color);

        [[nodiscard]] glm::vec3 GetPosition() const;
        void SetPosition(glm::vec3 position);

        [[nodiscard]] glm::vec3 GetDirection() const;
        void SetDirection(glm::vec3 direction);

        // Spot Extra Data X
        [[nodiscard]] float GetInnerSpotAngle() const;
        void SetInnerSpotAngle(float angle);

        // Spot Extra Data Y
        [[nodiscard]] float GetOuterSpotAngle() const;
        void SetOuterSpotAngle(float angle);

        [[nodiscard]] glm::mat4 GetShadowMatrix() const;
        void SetShadowMatrix(glm::mat4 shadowMatrix);
    };
} // Spinner

#endif //SPINNER_LIGHT_HPP
