#ifndef SPINNER_SWAPCHAIN_HPP
#define SPINNER_SWAPCHAIN_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanInstance.hpp"
#include "SwapchainSupportDetails.hpp"
#include "QueueFamilyIndices.hpp"

namespace Spinner
{

    class Swapchain
    {
    public:
        Swapchain() = default;
        ~Swapchain();

    protected:
        int Width = 0, Height = 0;
        bool VSync = false;

        vk::SwapchainKHR SwapchainKHR;
        std::vector<vk::Image> SwapchainImages;
        vk::Format SwapchainImageFormat = vk::Format::eUndefined;
        vk::Extent2D SwapchainExtent;
        std::vector<vk::ImageView> SwapchainImageViews;

    protected:
        [[nodiscard]] static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
        [[nodiscard]] vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availableModes) const;
        [[nodiscard]] vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;

    public:
        void Recreate(const vk::Device &device, const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface, int width, int height, bool vsync = false);

        [[nodiscard]] const vk::SwapchainKHR &GetSwapchainKHR() const;
        [[nodiscard]] vk::Extent2D GetSwapchainExtent() const;
        [[nodiscard]] vk::Image GetImage(uint32_t imageIndex);
        [[nodiscard]] vk::ImageView GetImageView(uint32_t imageIndex);
    };

} // Spinner

#endif //SPINNER_SWAPCHAIN_HPP
