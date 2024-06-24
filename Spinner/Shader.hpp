#ifndef SPINNER_SHADER_HPP
#define SPINNER_SHADER_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "Object.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

namespace Spinner
{
    class Graphics;

    class CommandBuffer;

    class ShaderInstance;

    struct ShaderCreateInfo
    {
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        DescriptorSetLayout::Pointer DescriptorSetLayout;
    };

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
        [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const;
        [[nodiscard]] std::vector<vk::PushConstantRange> GetPushConstantRanges() const;
        [[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

    protected:
        std::string ShaderName;
        vk::ShaderStageFlagBits ShaderStage;
        vk::ShaderStageFlags NextStage;
        vk::ShaderEXT VkShader;
        DescriptorSetLayout::Pointer DescriptorSetLayout;

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

    protected:
        Spinner::Shader::Pointer Shader;
        Spinner::DescriptorPool::Pointer DescriptorPool;
        vk::PipelineLayout VkPipelineLayout;
        std::vector<vk::DescriptorSet> VkDescriptorSets;

    public:
        static Pointer CreateInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool);
    };

    std::string GetShaderFileName(const std::string &shaderName, vk::ShaderStageFlagBits stage);

} // Spinner

#endif //SPINNER_SHADER_HPP
