#ifndef SPINNER_MATERIAL_HPP
#define SPINNER_MATERIAL_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "GLM.hpp"
#include "Texture.hpp"
#include "Constants.hpp"

namespace Spinner
{
    class ShaderInstance;

    class Material
    {
    public:
        using Pointer = std::shared_ptr<Material>;

        enum class DefaultTextureType
        {
            Black,
            White,
            Transparent,
            Magenta,
        };

        explicit Material(std::string materialName, glm::vec4 color = {1, 1, 1, 1}, float roughness = 0.5f, float metallic = 0.0f, float emissionStrength = 0.0f);
        virtual ~Material() = default;

        Pointer Duplicate();

        void ApplyMaterial(MeshConstants &constants);
        void ApplyTextures(std::shared_ptr<ShaderInstance> &shaderInstance);

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

        [[nodiscard]] Spinner::Texture::Pointer GetTexture(uint32_t textureIndex) const;
        void SetTexture(uint32_t textureIndex, Spinner::Texture::Pointer texture);
        [[nodiscard]] DefaultTextureType GetDefaultTextureType(uint32_t textureIndex) const;
        void SetDefaultTextureType(uint32_t textureIndex, DefaultTextureType defaultTextureType);

    protected:
        std::string Name;
        glm::vec4 Color{1, 1, 1, 1};
        float Roughness{0.5f};
        float Metallic{0.0f};
        float EmissionStrength{0.0f};
        std::array<float, CustomMaterialPropertyCount> CustomProperties{};
        std::array<Spinner::Texture::Pointer, MaxBoundTextures> Textures{};
        std::array<DefaultTextureType, MaxBoundTextures> DefaultTextureTypes{};

    public:
        static Pointer CreateMaterial(const std::string &materialName = "Material", glm::vec4 color = {1, 1, 1, 1}, float roughness = 0.5f, float metallic = 0.0f, float emissionStrength = 0.0f);
    };

} // Spinner

#endif //SPINNER_MATERIAL_HPP
