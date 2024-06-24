#include "DescriptorPool.hpp"

#include "VulkanInstance.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"

namespace Spinner
{
    DescriptorPool::DescriptorPool(const std::vector<vk::DescriptorPoolSize> &sizes, uint32_t maxSets) : Sizes(sizes), MaxSets(maxSets)
    {
        // Descriptor pool
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolCreateInfo.setMaxSets(MaxSets);
        descriptorPoolCreateInfo.setPoolSizes(Sizes);
        VkDescriptorPool = Graphics::GetDevice().createDescriptorPool(descriptorPoolCreateInfo);
    }

    DescriptorPool::~DescriptorPool()
    {
        if (VkDescriptorPool)
        {
            Graphics::GetDevice().destroyDescriptorPool(VkDescriptorPool);
            VkDescriptorPool = nullptr;
        }
    }

    DescriptorPool::Pointer DescriptorPool::CreateDefault()
    {
        constexpr uint32_t DefaultPoolSize = 1000u * MAX_FRAMES_IN_FLIGHT;
        constexpr uint32_t DefaultMaxSets = 1000u;

        static std::vector<vk::DescriptorPoolSize> sizes{
                {vk::DescriptorType::eSampler,              DefaultPoolSize},
                {vk::DescriptorType::eCombinedImageSampler, DefaultPoolSize},
                {vk::DescriptorType::eSampledImage,         DefaultPoolSize},
                {vk::DescriptorType::eStorageImage,         DefaultPoolSize},
                {vk::DescriptorType::eUniformTexelBuffer,   DefaultPoolSize},
                {vk::DescriptorType::eStorageTexelBuffer,   DefaultPoolSize},
                {vk::DescriptorType::eUniformBuffer,        DefaultPoolSize},
                {vk::DescriptorType::eUniformBufferDynamic, DefaultPoolSize},
                {vk::DescriptorType::eStorageBuffer,        DefaultPoolSize},
                {vk::DescriptorType::eStorageBufferDynamic, DefaultPoolSize},
                {vk::DescriptorType::eInputAttachment,      DefaultPoolSize},
        };

        return std::make_shared<Spinner::DescriptorPool>(sizes, DefaultMaxSets);
    }

    std::vector<vk::DescriptorSet> DescriptorPool::AllocateDescriptorSets(const std::shared_ptr<Shader> &shader)
    {
        return AllocateDescriptorSets(shader->GetDescriptorSetLayout());
    }

    std::vector<vk::DescriptorSet> DescriptorPool::AllocateDescriptorSets(vk::DescriptorSetLayout layout)
    {
        // Descriptor Sets
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.descriptorPool = VkDescriptorPool;
        allocateInfo.setSetLayouts(layout);

        return Graphics::GetDevice().allocateDescriptorSets(allocateInfo);
    }

    void DescriptorPool::FreeDescriptorSets(const vk::ArrayProxy<vk::DescriptorSet> &sets)
    {
        Graphics::GetDevice().freeDescriptorSets(VkDescriptorPool, sets);
    }
} // Spinner