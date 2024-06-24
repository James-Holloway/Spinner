#ifndef SPINNER_SWAPCHAINSUPPORTDETAILS_HPP
#define SPINNER_SWAPCHAINSUPPORTDETAILS_HPP

#include <vulkan/vulkan.hpp>

namespace Spinner
{

    struct SwapchainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR> PresentModes;

        SwapchainSupportDetails() = default;

        inline SwapchainSupportDetails(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface)
        {
            Capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
            Formats = physicalDevice.getSurfaceFormatsKHR(surface);
            PresentModes = physicalDevice.getSurfacePresentModesKHR(surface);
        };
    };

}

#endif //SPINNER_SWAPCHAINSUPPORTDETAILS_HPP
