#ifndef SPINNER_SHADER_HPP
#define SPINNER_SHADER_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "Object.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "VulkanInstance.hpp"

namespace Spinner
{
    class Graphics;

    class CommandBuffer;

    class ShaderInstance;

    class Buffer;

    class Texture;

    struct ShaderCreateInfo
    {
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        std::vector<DescriptorSetLayout::Pointer> DescriptorSetLayouts;
        DescriptorSetLayout::Pointer LightingDescriptorSetLayout;
    };

    // WARNING: Only takes push constants from the first descriptor set layout
    class Shader
    {
        friend class Graphics;

        friend class ShaderInstance;

        friend class CommandBuffer;

    public:
        using Pointer = std::shared_ptr<Shader>;

        explicit Shader(const ShaderCreateInfo &createInfo);
        ~Shader();
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
        [[nodiscard]] vk::DescriptorSetLayout GetLightingDescriptorSetLayout() const;
        [[nodiscard]] uint32_t GetLightingDescriptorSetIndex() const;
        [[nodiscard]] vk::PipelineLayout GetPipelineLayout() const;

        constexpr static const uint32_t InvalidBindingIndex = 0xFFFF'FFFF;

        uint32_t GetBindingFromIndex(uint32_t set, uint32_t indexOfType, vk::DescriptorType type);
        uint32_t GetBindingFromIndex(uint32_t set, uint32_t indexOfType, const std::vector<vk::DescriptorType> &types);

    protected:
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        vk::ShaderEXT VkShader;
        std::vector<DescriptorSetLayout::Pointer> DescriptorSetLayouts;
        DescriptorSetLayout::Pointer LightingDescriptorSetLayout;
        vk::PipelineLayout VkPipelineLayout;
        uint32_t LightingDescriptorSetIndex = InvalidBindingIndex;

    public:
        static Pointer CreateShader(const ShaderCreateInfo &createInfo);
        [[nodiscard]] static std::vector<Pointer> CreateLinkedShaders(const std::vector<ShaderCreateInfo> &createInfos);
    };

    struct ShaderInstanceCreateInfo
    {
        Shader::Pointer Shader;
        DescriptorPool::Pointer DescriptorPool;
    };

    class ShaderInstance
    {
        friend class Graphics;

        friend class Shader;

        friend class CommandBuffer;

    public:
        using Pointer = std::shared_ptr<ShaderInstance>;

        explicit ShaderInstance(const ShaderInstanceCreateInfo &createInfo);
        ShaderInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool);
        ~ShaderInstance();
        ShaderInstance(const ShaderInstance &other);
        ShaderInstance &operator=(const ShaderInstance &other);

    protected:
        void CreateDescriptors();

    public:
        [[nodiscard]] vk::ShaderStageFlagBits GetShaderStage() const;
        [[nodiscard]] vk::ShaderStageFlags GetNextStage() const;
        [[nodiscard]] Spinner::Shader::Pointer GetShader() const;
        [[nodiscard]] Spinner::DescriptorPool::Pointer GetDescriptorPool() const;

        [[nodiscard]] std::optional<vk::DescriptorType> GetDescriptorTypeOfBinding(uint32_t binding, uint32_t set = 0) const;
        [[nodiscard]] std::vector<vk::DescriptorSet> GetDescriptorSets(uint32_t currentFrame) const;
        [[nodiscard]] vk::DescriptorSet GetDescriptorSet(uint32_t currentFrame, uint32_t set = 0) const;
        void UpdateDescriptorBuffer(uint32_t currentFrame, uint32_t binding, const std::shared_ptr<Buffer> &buffer, uint32_t set = 0) const;
        void UpdateDescriptorImage(uint32_t currentFrame, uint32_t binding, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal, uint32_t set = 0) const;
        void UpdateDescriptorImage(uint32_t currentFrame, uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal, uint32_t set = 0) const;

    protected:
        Spinner::Shader::Pointer Shader;
        Spinner::DescriptorPool::Pointer DescriptorPool;
        std::array<std::vector<vk::DescriptorSet>, MAX_FRAMES_IN_FLIGHT> VkDescriptorSets;

    public:
        static Pointer CreateInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool);
    };

    std::string GetShaderFileName(const std::string &shaderName, vk::ShaderStageFlagBits stage);

} // Spinner

#endif //SPINNER_SHADER_HPP
