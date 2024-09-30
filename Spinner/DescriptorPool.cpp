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
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
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

    DescriptorPool::Pointer DescriptorPool::CreateDefault(uint32_t poolSize)
    {
        uint32_t DefaultPoolSize = poolSize * MAX_FRAMES_IN_FLIGHT;
        uint32_t DefaultMaxSets = poolSize;

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

    std::vector<vk::DescriptorSet> DescriptorPool::AllocateDescriptorSets(const std::shared_ptr<Shader> &shader) const
    {
        return AllocateDescriptorSets(shader->GetDescriptorSetLayouts());
    }

    std::vector<vk::DescriptorSet> DescriptorPool::AllocateDescriptorSets(const vk::ArrayProxy<vk::DescriptorSetLayout> &layouts) const
    {
        // Descriptor Sets
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.descriptorPool = VkDescriptorPool;
        allocateInfo.setSetLayouts(layouts);

        return Graphics::GetDevice().allocateDescriptorSets(allocateInfo);
    }

    void DescriptorPool::FreeDescriptorSets(const vk::ArrayProxy<vk::DescriptorSet> &sets) const
    {
        Graphics::GetDevice().freeDescriptorSets(VkDescriptorPool, sets);
    }

    void DescriptorPool::ResetPool() const
    {
        Graphics::GetDevice().resetDescriptorPool(VkDescriptorPool);
    }
} // Spinner