#include "Material.hpp"

#include <utility>
#include "Shader.hpp"
#include "Graphics.hpp"

namespace Spinner
{
    Material::Material(std::string materialName, glm::vec4 color, float roughness, float metallic, float emissionStrength) : Name(std::move(materialName)), Color(color), Roughness(roughness), Metallic(metallic), EmissionStrength(emissionStrength)
    {
        std::fill(DefaultTextureTypes.begin(), DefaultTextureTypes.end(), DefaultTextureType::Black);
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

    void Material::ApplyTextures(std::shared_ptr<ShaderInstance> &shaderInstance)
    {
        auto shader = shaderInstance->GetShader();
        for (uint32_t textureIndex = 0; textureIndex < MaxBoundTextures; textureIndex++)
        {
            auto &texture = Textures.at(textureIndex);
            uint32_t binding = shader->GetBindingFromIndex(textureIndex, vk::DescriptorType::eCombinedImageSampler);
            if (binding == Shader::InvalidBindingIndex)
            {
                // Return early if the binding could not be found from the index and type
                return;
            }
            else
            {
                if (texture == nullptr) // if no texture bound
                {
                    switch (DefaultTextureTypes.at(textureIndex))
                    {
                        case DefaultTextureType::Black:
                            texture = Texture::GetBlackTexture();
                            break;
                        case DefaultTextureType::White:
                            texture = Texture::GetWhiteTexture();
                            break;
                        case DefaultTextureType::Transparent:
                            texture = Texture::GetTransparentTexture();
                            break;
                        case DefaultTextureType::Magenta:
                            texture = Texture::GetMagentaTexture();
                            break;
                    }
                }

                shaderInstance->UpdateDescriptorImage(Graphics::GetCurrentFrame(), binding, texture, vk::ImageLayout::eShaderReadOnlyOptimal);
            }
        }
    }

    Material::Pointer Material::Duplicate()
    {
        auto material = std::make_shared<Material>(Name, Color, Roughness, Metallic, EmissionStrength);
        // Copy custom properties
        std::copy(CustomProperties.begin(), CustomProperties.end(), material->CustomProperties.begin());
        // Copy texture pointers
        std::copy(Textures.begin(), Textures.end(), material->Textures.begin());

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

    Spinner::Texture::Pointer Material::GetTexture(uint32_t textureIndex) const
    {
        return Textures.at(textureIndex);
    }

    void Material::SetTexture(uint32_t textureIndex, Spinner::Texture::Pointer texture)
    {
        Textures.at(textureIndex) = std::move(texture);
    }

    Material::DefaultTextureType Material::GetDefaultTextureType(uint32_t textureIndex) const
    {
        return DefaultTextureTypes.at(textureIndex);
    }

    void Material::SetDefaultTextureType(uint32_t textureIndex, Material::DefaultTextureType defaultTextureType)
    {
        DefaultTextureTypes.at(textureIndex) = defaultTextureType;
    }
} // Spinner