#ifndef SPINNER_SHADER_HPP
#define SPINNER_SHADER_HPP

#include <vulkan/vulkan.hpp>
#include <memory>

#include "CommandBuffer.hpp"
#include "Object.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "VulkanInstance.hpp"

namespace Spinner
{
    class Graphics;
    class DrawCommand;
    class CommandBuffer;
    class ShaderGroup;
    class Buffer;
    class Texture;

    namespace Components
    {
        class Component;
    }

    struct ShaderCreateInfo
    {
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        std::vector<DescriptorSetLayout::Pointer> DescriptorSetLayouts;
        DescriptorSetLayout::Pointer SceneDescriptorSetLayout;
        DescriptorSetLayout::Pointer LightingDescriptorSetLayout;
        std::function<void(const std::shared_ptr<Spinner::DrawCommand> &, Components::Component *)> UpdateDrawComponentCallback;
    };

    // WARNING: Only takes push constants from the first descriptor set layout
    class Shader final : public Object
    {
        friend class Graphics;
        friend class ShaderGroup;
        friend class CommandBuffer;

    public:
        using Pointer = std::shared_ptr<Shader>;

        explicit Shader(const ShaderCreateInfo &createInfo);
        ~Shader() override;
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        [[nodiscard]] std::string GetShaderName() const;
        [[nodiscard]] vk::ShaderStageFlagBits GetShaderStage() const;
        [[nodiscard]] vk::ShaderStageFlags GetNextStage() const;
        [[nodiscard]] vk::ShaderEXT GetVkShader() const;
        [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings(uint32_t index) const;
        [[nodiscard]] std::vector<vk::PushConstantRange> GetPushConstantRanges(uint32_t index) const;
        [[nodiscard]] vk::DescriptorSetLayout GetDescriptorSetLayout(uint32_t index) const;
        [[nodiscard]] std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts() const;
        [[nodiscard]] vk::PipelineLayout GetPipelineLayout() const;
        [[nodiscard]] std::optional<vk::DescriptorType> GetDescriptorTypeOfBinding(uint32_t binding, uint32_t set = 0) const;

        [[nodiscard]] vk::DescriptorSetLayout GetSceneDescriptorSetLayout() const;
        [[nodiscard]] vk::DescriptorSetLayout GetLightingDescriptorSetLayout() const;
        [[nodiscard]] uint32_t GetSceneDescriptorSetIndex() const;
        [[nodiscard]] uint32_t GetLightingDescriptorSetIndex() const;

        constexpr static const uint32_t InvalidBindingIndex = 0xFFFF'FFFF;

        uint32_t GetBindingFromIndex(uint32_t set, uint32_t indexOfType, vk::DescriptorType type);
        uint32_t GetBindingFromIndex(uint32_t set, uint32_t indexOfType, const std::vector<vk::DescriptorType> &types);

    protected:
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        vk::ShaderEXT VkShader;
        std::vector<DescriptorSetLayout::Pointer> DescriptorSetLayouts;
        DescriptorSetLayout::Pointer SceneDescriptorSetLayout;
        DescriptorSetLayout::Pointer LightingDescriptorSetLayout;
        vk::PipelineLayout VkPipelineLayout;
        uint32_t SceneDescriptorSetIndex = InvalidBindingIndex;
        uint32_t LightingDescriptorSetIndex = InvalidBindingIndex;

    public:
        Callback<const std::shared_ptr<Spinner::DrawCommand> &, Components::Component *> UpdateDrawComponentCallback;

    public:
        // You probably want to create a ShaderGroup instead of an unlinked shader
        static Pointer CreateShader(const ShaderCreateInfo &createInfo);
    };

    class ShaderGroup final
    {
        friend class Graphics;
        friend class Shader;

    public:
        using Pointer = std::shared_ptr<ShaderGroup>;

        ShaderGroup() = default;
        ~ShaderGroup() = default;

    protected:
        std::vector<Shader::Pointer> Shaders;

    public:
        void BindShaders(const std::shared_ptr<CommandBuffer> &commandBuffer) const;
        void RunUpdateDrawComponentCallbacks(const std::shared_ptr<Spinner::DrawCommand> &drawCommand, Components::Component *drawComponent);

        [[nodiscard]] bool HasShaderStage(vk::ShaderStageFlagBits shaderStage) const;
        [[nodiscard]] Shader::Pointer GetShader(vk::ShaderStageFlagBits shaderStage) const;

    public:
        [[nodiscard]] static Spinner::ShaderGroup::Pointer CreateShaderGroup(const std::vector<ShaderCreateInfo> &createInfos);
    };

    std::string GetShaderFileName(const std::string &shaderName, vk::ShaderStageFlagBits stage);
} // Spinner

#endif //SPINNER_SHADER_HPP
