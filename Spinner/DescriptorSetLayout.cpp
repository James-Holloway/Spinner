#include "DescriptorSetLayout.hpp"

#include "Graphics.hpp"

namespace Spinner
{
    DescriptorSetLayout::DescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges) : DescriptorSetLayoutBindings(layoutBindings), PushConstantRanges(pushConstantRanges)
    {
        // Descriptor set layout
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo.setBindings(DescriptorSetLayoutBindings);
        VkDescriptorSetLayout = Graphics::GetDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (VkDescriptorSetLayout)
        {
            Graphics::GetDevice().destroyDescriptorSetLayout(VkDescriptorSetLayout);
        }
    }

    const vk::DescriptorSetLayout &DescriptorSetLayout::GetDescriptorSetLayout() const
    {
        return VkDescriptorSetLayout;
    }

    std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayout::GetDescriptorSetLayoutBindings() const
    {
        return DescriptorSetLayoutBindings;
    }

    std::vector<vk::PushConstantRange> DescriptorSetLayout::GetPushConstantRanges() const
    {
        return PushConstantRanges;
    }

    DescriptorSetLayout::Pointer DescriptorSetLayout::CreateDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges)
    {
        return std::make_shared<Spinner::DescriptorSetLayout>(layoutBindings, pushConstantRanges);
    }

} // Spinner