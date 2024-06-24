#ifndef SPINNER_VULKANUTILITIES_HPP
#define SPINNER_VULKANUTILITIES_HPP

#include <vulkan/vulkan.hpp>

namespace Spinner
{
    size_t VkFormatByteWidth(vk::Format format);

    template<typename T>
    inline vk::IndexType GetVkIndexType()
    {
        throw std::runtime_error("Unknown index type");
    }

    template<>
    inline vk::IndexType GetVkIndexType<uint16_t>()
    { return vk::IndexType::eUint16; };

    template<>
    inline vk::IndexType GetVkIndexType<uint32_t>()
    { return vk::IndexType::eUint32; };
} // Spinner

#endif //SPINNER_VULKANUTILITIES_HPP
