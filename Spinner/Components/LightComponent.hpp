#ifndef SPINNER_LIGHTCOMPONENT_HPP
#define SPINNER_LIGHTCOMPONENT_HPP

#include "Component.hpp"
#include "../Light.hpp"
#include "../Image.hpp"
#include "../Buffer.hpp"
#include "../DescriptorPool.hpp"

namespace Spinner
{
    class Scene;
    class DrawManager;

    namespace Components
    {
        class LightComponent : public Component
        {
        public:
            constexpr static vk::Format ShadowMapFormat = vk::Format::eD16Unorm;
            constexpr static uint32_t ShadowMapWidth = 1024;
            constexpr static vk::ImageUsageFlags ShadowMapUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eDepthStencilAttachment;

            LightComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            float InnerSpotAngle = glm::radians(35.0f);
            float OuterSpotAngle = glm::radians(45.0f);
            Spinner::LightType LightType = Spinner::LightType::None;
            glm::vec3 LightColor = {1.0f, 1.0f, 1.0f};
            float LightStrength = 1.0f;
            bool IsShadowCaster = true;

            std::vector<Spinner::Buffer::Pointer> ShadowSceneBuffers; // 6 in the case of point shadow, 1 otherwise
            Image::Pointer ShadowMapImage = nullptr;
            vk::ImageView ShadowMapImageView = nullptr; // Depth rendering ImageView
            std::array<vk::ImageView, 6> ShadowCubeMapImageView;

        public:
            [[nodiscard]] Spinner::LightType GetLightType() const;
            void SetLightType(Spinner::LightType lightType);

            [[nodiscard]] glm::vec3 GetLightColor() const;
            void SetLightColor(glm::vec3 lightColor);
            [[nodiscard]] float GetLightStrength() const;
            void SetLightStrength(float strength);

            // Both inner and outer spot angles are stored in radians
            [[nodiscard]] float GetInnerSpotAngle() const;
            void SetInnerSpotAngle(float angle);
            [[nodiscard]] float GetOuterSpotAngle() const;
            void SetOuterSpotAngle(float angle);

            [[nodiscard]] bool GetIsShadowCaster() const;
            void SetIsShadowCaster(bool isShadowCaster);

            [[nodiscard]] Spinner::Light GetLight() const;

            [[nodiscard]] Spinner::Image::Pointer GetShadowMapImage() const;
            [[nodiscard]] vk::ImageView GetShadowMapImageDepthView() const;

            [[nodiscard]] glm::mat4 GetShadowProjectionMatrix() const;
            [[nodiscard]] glm::mat4 GetShadowViewMatrix() const;

            void RenderShadow(CommandBuffer::Pointer &commandBuffer, const Spinner::DescriptorPool::Pointer &descriptorPool);

            void RenderDebugUI();

        protected:
            void RenderShadowFace(uint32_t faceIndex, CommandBuffer::Pointer &commandBuffer, const Spinner::DescriptorPool::Pointer &descriptorPool);
            void SetupShadows(vk::Extent2D shadowTextureSize);
        };

        template<>
        inline constexpr ComponentId GetComponentId<LightComponent>()
        {
            return 4;
        }

        template<>
        inline const char *GetComponentName<LightComponent>()
        {
            return "LightComponent";
        }

        template<>
        inline void RenderDebugUI<LightComponent>(LightComponent *component)
        {
            component->RenderDebugUI();
        }
    } // Components
} // Spinner

#endif //SPINNER_LIGHTCOMPONENT_HPP
