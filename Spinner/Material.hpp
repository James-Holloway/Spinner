#ifndef SPINNER_MATERIAL_HPP
#define SPINNER_MATERIAL_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "GLM.hpp"
#include "Image.hpp"
#include "Constants.hpp"

namespace Spinner
{
    class Material
    {
    public:
        using Pointer = std::shared_ptr<Material>;

        explicit Material(std::string materialName, glm::vec4 color = {1, 1, 1, 1}, float roughness = 0.5f, float metallic = 0.0f, float emissionStrength = 0.0f);
        virtual ~Material() = default;

        Pointer Duplicate();

        void ApplyMaterial(MeshConstants &constants);

        [[nodiscard]] std::string GetName() const;
        void SetName(const std::string &name);
        [[nodiscard]] glm::vec4 GetColor() const;
        void SetColor(glm::vec4 color);
        [[nodiscard]] float GetRoughness() const;
        void SetRoughness(float roughness);
        [[nodiscard]] float GetMetallic() const;
        void SetMetallic(float metallic);
        [[nodiscard]] float GetEmissionStrength() const;
        void SetEmissionStrength(float emissionStrength);
        [[nodiscard]] float GetCustomProperty(size_t index);
        void SetCustomProperty(size_t index, float customProperty);

    protected:
        std::string Name;
        glm::vec4 Color{1, 1, 1, 1};
        float Roughness{0.5f};
        float Metallic{0.0f};
        float EmissionStrength{0.0f};
        std::array<float, CustomMaterialPropertyCount> CustomProperties{};

    public:
        static Pointer CreateMaterial(const std::string &materialName = "Material", glm::vec4 color = {1, 1, 1, 1}, float roughness = 0.5f, float metallic = 0.0f, float emissionStrength = 0.0f);
    };

} // Spinner

#endif //SPINNER_MATERIAL_HPP
