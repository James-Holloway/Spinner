#ifndef SPINNER_SCENEDESCRIPTORS_HPP
#define SPINNER_SCENEDESCRIPTORS_HPP

#include <vulkan/vulkan.hpp>
#include "DescriptorSetLayout.hpp"

namespace Spinner
{
    class SceneDescriptors
    {
    public:
        static std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings();
        [[nodiscard]] static DescriptorSetLayout::Pointer GetDescriptorSetLayout();
    };
}

#endif //SPINNER_SCENEDESCRIPTORS_HPP
