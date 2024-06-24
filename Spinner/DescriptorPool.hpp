#ifndef SPINNER_DESCRIPTORPOOL_HPP
#define SPINNER_DESCRIPTORPOOL_HPP

#include <vulkan/vulkan.hpp>
#include <memory>

namespace Spinner
{
    class ShaderInstance;

    class Shader;

    class DescriptorPool
    {
        friend class ShaderInstance;

    public:
        using Pointer = std::shared_ptr<DescriptorPool>;

        DescriptorPool(const std::vector<vk::DescriptorPoolSize> &sizes, uint32_t maxSets);
        ~DescriptorPool();

        std::vector<vk::DescriptorSet> AllocateDescriptorSets(const std::shared_ptr<Shader> &shader);
        std::vector<vk::DescriptorSet> AllocateDescriptorSets(vk::DescriptorSetLayout layout);
        void FreeDescriptorSets(const vk::ArrayProxy<vk::DescriptorSet> &sets);

    public:
        vk::DescriptorPool VkDescriptorPool;
        std::vector<vk::DescriptorPoolSize> Sizes;
        uint32_t MaxSets;

    public:
        static Pointer CreateDefault();
    };

} // Spinner

#endif //SPINNER_DESCRIPTORPOOL_HPP
