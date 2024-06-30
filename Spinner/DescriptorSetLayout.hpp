#ifndef SPINNER_DESCRIPTORSETLAYOUT_HPP
#define SPINNER_DESCRIPTORSETLAYOUT_HPP

#include <memory>
#include <vulkan/vulkan.hpp>

namespace Spinner
{

    class DescriptorSetLayout
    {
    public:
        using Pointer = std::shared_ptr<DescriptorSetLayout>;

        explicit DescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges = {}, const std::vector<vk::DescriptorBindingFlags> &layoutBindingFlags = {});
        ~DescriptorSetLayout();

        [[nodiscard]] const vk::DescriptorSetLayout &GetDescriptorSetLayout() const;
        [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const;
        [[nodiscard]] std::vector<vk::PushConstantRange> GetPushConstantRanges() const;

    protected:
        vk::DescriptorSetLayout VkDescriptorSetLayout;
        std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBindings;
        std::vector<vk::PushConstantRange> PushConstantRanges;
        std::vector<vk::DescriptorBindingFlags> DescriptorSetLayoutBindingFlags;

    public:
        [[nodiscard]] static Pointer CreateDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges = {}, const std::vector<vk::DescriptorBindingFlags> &layoutBindingFlags = {});
    };

} // Spinner

#endif //SPINNER_DESCRIPTORSETLAYOUT_HPP
