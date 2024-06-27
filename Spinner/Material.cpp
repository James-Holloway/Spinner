#include "Material.hpp"

#include <utility>

namespace Spinner
{
    Material::Material(std::string materialName, glm::vec4 color, float roughness, float metallic, float emissionStrength) : Name(std::move(materialName)), Color(color), Roughness(roughness), Metallic(metallic), EmissionStrength(emissionStrength)
    {

    }

    Material::Pointer Material::CreateMaterial(const std::string &materialName, glm::vec4 color, float roughness, float metallic, float emissionStrength)
    {
        return std::make_shared<Spinner::Material>(materialName, color, roughness, metallic, emissionStrength);
    }

    void Material::ApplyMaterial(MeshConstants &constants)
    {
        constants.MaterialColor = Color;
        constants.MaterialProperties = {Roughness, Metallic, EmissionStrength, 0.0f};
        for (size_t i = 0; i < CustomProperties.size(); i++)
        {
            constants.CustomMaterialProperties[i] = CustomProperties[i];
        }
    }

    Material::Pointer Material::Duplicate()
    {
        auto material = std::make_shared<Material>(Name, Color, Roughness, Metallic, EmissionStrength);
        // Copy custom properties
        std::copy(CustomProperties.begin(), CustomProperties.end(), material->CustomProperties.begin());

        return material;
    }

    std::string Material::GetName() const
    {
        return Name;
    }

    void Material::SetName(const std::string &name)
    {
        Name = name;
    }

    glm::vec4 Material::GetColor() const
    {
        return Color;
    }

    void Material::SetColor(glm::vec4 color)
    {
        Color = color;
    }

    float Material::GetRoughness() const
    {
        return Roughness;
    }

    void Material::SetRoughness(float roughness)
    {
        Roughness = roughness;
    }

    float Material::GetMetallic() const
    {
        return Metallic;
    }

    void Material::SetMetallic(float metallic)
    {
        Metallic = metallic;
    }

    float Material::GetEmissionStrength() const
    {
        return EmissionStrength;
    }

    void Material::SetEmissionStrength(float emissionStrength)
    {
        EmissionStrength = emissionStrength;
    }

    float Material::GetCustomProperty(size_t index)
    {
        return CustomProperties.at(index);
    }

    void Material::SetCustomProperty(size_t index, float customProperty)
    {
        CustomProperties.at(index) = customProperty;
    }
} // Spinner