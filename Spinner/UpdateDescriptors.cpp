#include "UpdateDescriptors.hpp"

#include "Graphics.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"

namespace Spinner
{
    void UpdateDescriptors::UpdateDescriptorBuffer(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, const std::shared_ptr<Buffer> &buffer)
    {
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = buffer->VkBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = vk::WholeSize;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    void UpdateDescriptors::UpdateDescriptorImage(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout)
    {
        assert(texture != nullptr);
        assert(texture->GetImage() != nullptr);
        assert(texture->GetImage()->GetMainImageView() != nullptr);
        assert(texture->GetSampler() != nullptr);

        UpdateDescriptorImage(set, binding, descriptorType, texture->GetImage()->GetMainImageView(), texture->GetSampler()->GetSampler(), imageLayout);
    }

    void UpdateDescriptors::UpdateDescriptorImage(vk::DescriptorSet set, uint32_t binding, vk::DescriptorType descriptorType, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = imageLayout;
        imageInfo.sampler = sampler;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = set;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }
} // Spinner