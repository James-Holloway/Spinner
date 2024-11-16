#ifndef SPINNER_SAMPLER_HPP
#define SPINNER_SAMPLER_HPP

#include <vulkan/vulkan.hpp>
#include <memory>

namespace Spinner
{
    class Sampler
    {
    public:
        using Pointer = std::shared_ptr<Sampler>;

        explicit Sampler(vk::Filter minMagFilter = vk::Filter::eLinear, vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat, float maxAnisotropy = 8.0f, std::optional<vk::CompareOp> compareOp = {});
        virtual ~Sampler();

        [[nodiscard]] vk::Sampler GetSampler() const;

    protected:
        vk::Sampler VkSampler;

    public:
        static Pointer CreateSampler(vk::Filter minMagFilter = vk::Filter::eLinear, vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat, float maxAnisotropy = 8.0f, std::optional<vk::CompareOp> compareOp = {});
    };
} // Spinner

#endif //SPINNER_SAMPLER_HPP
