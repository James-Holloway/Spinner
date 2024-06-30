#ifndef SPINNER_LIGHTING_HPP
#define SPINNER_LIGHTING_HPP

#include <memory>
#include "Light.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"

namespace Spinner
{

    namespace Components
    {
        class LightComponent;
    }

    struct LightInfo
    {
        uint32_t LightCount;
        uint32_t ShadowCount;
        glm::vec2 Padding;
    };

    class Lighting
    {
    public:
        using Pointer = std::shared_ptr<Lighting>;

        constexpr static uint32_t DefaultLightCount = 32;
        constexpr static uint32_t DefaultShadowCount = 16;

        explicit Lighting(uint32_t lightCount = DefaultLightCount, uint32_t shadowCount = DefaultShadowCount);

        void UpdateLights(glm::vec3 viewerPosition, const std::vector<Components::LightComponent *> &lightComponents);
        void UpdateDescriptors(vk::DescriptorSet set, bool shadowsOnly = false);

    protected:
        Buffer::Pointer LightInfoBuffer;
        Buffer::Pointer LightBuffer;

        std::vector<Texture::Pointer> ShadowTextures;

        const uint32_t MaxLightCount = 0;
        const uint32_t MaxShadowCount = 0;

    protected:
        static std::weak_ptr<Lighting> GlobalLighting;

    public:
        static std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings(uint32_t shadowCount = DefaultShadowCount);

        static std::vector<vk::DescriptorBindingFlags> GetDescriptorBindingFlags(uint32_t shadowCount = DefaultShadowCount);
        static Pointer CreateLighting(uint32_t lightCount = DefaultLightCount, uint32_t shadowCount = DefaultShadowCount);
    };

} // Spinner

#endif //SPINNER_LIGHTING_HPP
