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

        [[nodiscard]] std::vector<vk::DescriptorSet> AllocateDescriptorSets(const std::shared_ptr<Shader> &shader) const;
        [[nodiscard]] std::vector<vk::DescriptorSet> AllocateDescriptorSets(const vk::ArrayProxy<vk::DescriptorSetLayout> &layouts) const;
        void ResetPool() const;

    public:
        vk::DescriptorPool VkDescriptorPool;
        std::vector<vk::DescriptorPoolSize> Sizes;
        uint32_t MaxSets;

    public:
        static Pointer CreateDefault(uint32_t poolSize = 1000u);
    };

} // Spinner

#endif //SPINNER_DESCRIPTORPOOL_HPP
