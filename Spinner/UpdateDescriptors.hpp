#ifndef SPINNER_UPDATEDESCRIPTORS_HPP
#define SPINNER_UPDATEDESCRIPTORS_HPP

#include <vulkan/vulkan.hpp>
#include <memory>

namespace Spinner
{
    class Buffer;

    class Texture;

    namespace UpdateDescriptors
    {
        void UpdateDescriptorBuffer(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, const std::shared_ptr<Buffer> &buffer);
        void UpdateDescriptorImage(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
        void UpdateDescriptorImage(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
    };

} // Spinner

#endif //SPINNER_UPDATEDESCRIPTORS_HPP
