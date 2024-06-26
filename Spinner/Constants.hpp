#ifndef SPINNER_CONSTANTS_HPP
#define SPINNER_CONSTANTS_HPP

#include "GLM.hpp"

namespace Spinner
{
    struct MeshConstants
    {
        alignas(16) glm::mat4 Model{1.0f};
        alignas(16) glm::vec4 MaterialColor = {1.0f, 1.0f, 1.0f, 1.0f}; // Red, Green, Blue, Alpha
        alignas(16) glm::vec4 MaterialProperties = {0.5f, 0.0f, 0.0f, 0.0f}; // Roughness, Metallic, Emission Strength, Unused

        [[nodiscard]] inline glm::vec4 GetColor() const
        {
            return MaterialColor;
        }

        inline void SetColor(glm::vec4 color)
        {
            MaterialColor = color;
        }

        [[nodiscard]] inline float GetRoughness() const
        {
            return MaterialProperties.x;
        }

        inline void SetRoughness(float roughness)
        {
            MaterialProperties.x = roughness;
        }

        [[nodiscard]] inline float GetMetallic() const
        {
            return MaterialProperties.y;
        }

        inline void SetMetallic(float metallic)
        {
            MaterialProperties.y = metallic;
        }

        [[nodiscard]] inline float GetEmissionStrength() const
        {
            return MaterialProperties.z;
        }

        inline void SetEmissionStrength(float emission)
        {
            MaterialProperties.z = emission;
        }
    };

    struct SceneConstants
    {
        alignas(16) glm::mat4 ViewProjection{1.0f};
        alignas(16) glm::mat4 View{1.0f};
        alignas(16) glm::mat4 Projection{1.0f};
        alignas(16) glm::vec3 CameraPosition{0, 0, 0};
        alignas(4) glm::vec2 CameraExtent{1, 1};
        alignas(4) float Time{0};
        alignas(4) float DeltaTime{0};
    };

} // Spinner

#endif //SPINNER_CONSTANTS_HPP
