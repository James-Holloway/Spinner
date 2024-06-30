#include "DescriptorSetLayout.hpp"

#include "Graphics.hpp"

namespace Spinner
{
    DescriptorSetLayout::DescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges, const std::vector<vk::DescriptorBindingFlags> &layoutBindingFlags) : DescriptorSetLayoutBindings(layoutBindings), PushConstantRanges(pushConstantRanges), DescriptorSetLayoutBindingFlags(layoutBindingFlags)
    {
        vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags;
        bindingFlags.setBindingFlags(layoutBindingFlags);

        // Descriptor set layout
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo.setBindings(DescriptorSetLayoutBindings);

        if (!layoutBindingFlags.empty())
        {
            descriptorSetLayoutCreateInfo.setPNext(&bindingFlags);
            for (auto flag : layoutBindingFlags)
            {
                if ((flag & vk::DescriptorBindingFlagBits::eUpdateAfterBind) != vk::DescriptorBindingFlags{})
                {
                    descriptorSetLayoutCreateInfo.flags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
                }
            }
        }

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

    DescriptorSetLayout::Pointer DescriptorSetLayout::CreateDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings, const std::vector<vk::PushConstantRange> &pushConstantRanges, const std::vector<vk::DescriptorBindingFlags> &layoutBindingFlags)
    {
        return std::make_shared<Spinner::DescriptorSetLayout>(layoutBindings, pushConstantRanges, layoutBindingFlags);
    }

} // Spinner