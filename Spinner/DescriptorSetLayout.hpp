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

        explicit DescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges = {});
        ~DescriptorSetLayout();

        [[nodiscard]] const vk::DescriptorSetLayout &GetDescriptorSetLayout() const;
        [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const;
        [[nodiscard]] std::vector<vk::PushConstantRange> GetPushConstantRanges() const;

    protected:
        vk::DescriptorSetLayout VkDescriptorSetLayout;
        std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBindings;
        std::vector<vk::PushConstantRange> PushConstantRanges;

    public:
        static Pointer CreateDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges = {});
    };

} // Spinner

#endif //SPINNER_DESCRIPTORSETLAYOUT_HPP
