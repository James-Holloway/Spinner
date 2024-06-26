#include "VulkanUtilities.hpp"


namespace Spinner
{

#define CASES_VK_FORMAT_FAMILY(family)\
    case vk::Format::e##family##Unorm:\
    case vk::Format::e##family##Snorm:\
    case vk::Format::e##family##Uscaled:\
    case vk::Format::e##family##Uint:\
    case vk::Format::e##family##Sint:

#define CASES_VK_FORMAT_FAMILY_FLOAT(family)\
    case vk::Format::e##family##Unorm:\
    case vk::Format::e##family##Snorm:\
    case vk::Format::e##family##Uscaled:\
    case vk::Format::e##family##Uint:\
    case vk::Format::e##family##Sint:\
    case vk::Format::e##family##Sfloat:

#define CASES_VK_FORMAT_FAMILY_BIG(family)\
    case vk::Format::e##family##Uint:\
    case vk::Format::e##family##Sint:\
    case vk::Format::e##family##Sfloat:

#define CASES_VK_FORMAT_FAMILY_PACK32(family)\
    case vk::Format::e##family##UnormPack32:\
    case vk::Format::e##family##SnormPack32:\
    case vk::Format::e##family##UscaledPack32:\
    case vk::Format::e##family##SscaledPack32:\
    case vk::Format::e##family##SintPack32:\
    case vk::Format::e##family##UintPack32:

    size_t VkFormatByteWidth(vk::Format format)
    {
        switch (format)
        {
            default:
                throw std::runtime_error("Unhandled format byte width size");
            case vk::Format::eUndefined:
                return 0;
            CASES_VK_FORMAT_FAMILY(R8)
            case vk::Format::eR8Srgb:
            case vk::Format::eS8Uint:
                return 1;
            CASES_VK_FORMAT_FAMILY(R8G8)
            case vk::Format::eR8G8Srgb:
            CASES_VK_FORMAT_FAMILY_FLOAT(R16)
            case vk::Format::eD16Unorm:
                return 2;
            CASES_VK_FORMAT_FAMILY(R8G8B8)
            case vk::Format::eR8G8B8Srgb:
                return 3;
            CASES_VK_FORMAT_FAMILY(R8G8B8A8)
            case vk::Format::eR8G8B8A8Srgb:
            CASES_VK_FORMAT_FAMILY_FLOAT(R16G16)
            CASES_VK_FORMAT_FAMILY_BIG(R32)
            CASES_VK_FORMAT_FAMILY(B8G8R8A8)
            CASES_VK_FORMAT_FAMILY_PACK32(A2R10G10B10)
            CASES_VK_FORMAT_FAMILY_PACK32(A2B10G10R10)
            case vk::Format::eA8B8G8R8SrgbPack32:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eX8D24UnormPack32:
            case vk::Format::eD32Sfloat:
                return 4;
            CASES_VK_FORMAT_FAMILY_FLOAT(R16G16B16)
                return 6;
            CASES_VK_FORMAT_FAMILY_FLOAT(R16G16B16A16)
            CASES_VK_FORMAT_FAMILY_BIG(R32G32)
            CASES_VK_FORMAT_FAMILY_BIG(R64)
            case vk::Format::eD32SfloatS8Uint:
                return 8;
            CASES_VK_FORMAT_FAMILY_BIG(R32G32B32)
                return 12;
            CASES_VK_FORMAT_FAMILY_BIG(R32G32B32A32)
            CASES_VK_FORMAT_FAMILY_BIG(R64G64)
                return 16;
            CASES_VK_FORMAT_FAMILY_BIG(R64G64B64)
                return 24;
            CASES_VK_FORMAT_FAMILY_BIG(R64G64B64A64)
                return 32;
        }
    }

    bool VkFormatHasStencilComponent(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD16UnormS8Uint || format == vk::Format::eS8Uint;
    }
} // Spinner