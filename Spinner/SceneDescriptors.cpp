#include "SceneDescriptors.hpp"

namespace Spinner
{
    std::vector<vk::DescriptorSetLayoutBinding> SceneDescriptors::GetDescriptorSetLayoutBindings()
    {
        return std::vector<vk::DescriptorSetLayoutBinding>{
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
        };
    }

    DescriptorSetLayout::Pointer SceneDescriptors::GetDescriptorSetLayout()
    {
        static std::weak_ptr<DescriptorSetLayout> WeakDescriptorSetLayout{};
        if (WeakDescriptorSetLayout.expired())
        {
            auto descriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(GetDescriptorSetLayoutBindings(), {}, {});
            WeakDescriptorSetLayout = descriptorSetLayout;
            return descriptorSetLayout;
        }

        return WeakDescriptorSetLayout.lock();
    }
} // Spinner