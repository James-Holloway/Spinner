#include "Sampler.hpp"

#include "Graphics.hpp"

namespace Spinner
{
    Sampler::Sampler(vk::Filter minMagFilter, vk::SamplerMipmapMode mipFilter, vk::SamplerAddressMode repeat, float maxAnisotropy)
    {
        float maxHardwareAnisotropy = Graphics::GetPhysicalDevice().getProperties().limits.maxSamplerAnisotropy;
        if (maxHardwareAnisotropy < maxAnisotropy)
        {
            maxAnisotropy = maxHardwareAnisotropy;
        }

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.minFilter = samplerInfo.magFilter = minMagFilter;
        samplerInfo.mipmapMode = mipFilter;
        samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = repeat;
        samplerInfo.maxAnisotropy = maxAnisotropy;
        samplerInfo.anisotropyEnable = maxAnisotropy > 0;
        samplerInfo.compareEnable = false;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.unnormalizedCoordinates = false;

        VkSampler = Graphics::GetDevice().createSampler(samplerInfo);
    }

    Sampler::~Sampler()
    {
        if (VkSampler)
        {
            Graphics::GetDevice().destroySampler(VkSampler);
        }
    }

    Sampler::Pointer Sampler::CreateSampler(vk::Filter minMagFilter, vk::SamplerMipmapMode mipFilter, vk::SamplerAddressMode repeat, float maxAnisotropy)
    {
        return std::make_shared<Spinner::Sampler>(minMagFilter, mipFilter, repeat, maxAnisotropy);
    }

    vk::Sampler Sampler::GetSampler() const
    {
        return VkSampler;
    }


} // Spinner