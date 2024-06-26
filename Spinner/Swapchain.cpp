#include "Swapchain.hpp"
#include "Graphics.hpp"

namespace Spinner
{
    Swapchain::~Swapchain()
    {
        auto &device = Graphics::GetDevice();

        // Destroy image views
        for (auto &imageView : SwapchainImageViews)
        {
            if (imageView)
            {
                device.destroyImageView(imageView);
            }
        }
        // SwapchainImages do not need to be destroyed
        if (SwapchainKHR)
        {
            device.destroySwapchainKHR(SwapchainKHR);
        }
    }

    vk::SurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm)
            {
                return availableFormat;
            }
        }

        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eR8G8B8A8Unorm)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availableModes) const
    {
        if (!VSync)
        {
            for (const auto &availablePresentMode : availableModes)
            {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                {
                    return availablePresentMode;
                }
            }
            for (const auto &availablePresentMode : availableModes)
            {
                if (availablePresentMode == vk::PresentModeKHR::eImmediate)
                {
                    return availablePresentMode;
                }
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        vk::Extent2D actualExtent = {
                static_cast<uint32_t>(Width),
                static_cast<uint32_t>(Height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void Swapchain::Recreate(const vk::Device &device, const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface, int width, int height, bool vsync)
    {
        // Destroy if exists first
        for (auto &imageView : SwapchainImageViews)
        {
            if (imageView)
            {
                device.destroyImageView(imageView);
            }
        }
        // SwapchainImages do not need to be destroyed
        if (SwapchainKHR)
        {
            device.destroySwapchainKHR(SwapchainKHR);
        }

        // Creation
        Width = width;
        Height = height;
        VSync = vsync;

        SwapchainSupportDetails details{physicalDevice, surface};

        vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.Formats);
        vk::PresentModeKHR presentMode = ChooseSwapPresentMode(details.PresentModes);
        vk::Extent2D extent = ChooseSwapExtent(details.Capabilities);

        // Increase image count by one so we don't have to wait on the driver, but ensure it is still smaller than maxImageCount
        uint32_t imageCount = details.Capabilities.minImageCount + 1;
        if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
        {
            imageCount = details.Capabilities.maxImageCount;
        }
        ImageCount = imageCount;

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        QueueFamilyIndices indices = Graphics::FindQueueFamilies(physicalDevice, surface);
        std::vector<uint32_t> queueFamilyIndices = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};
        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.setQueueFamilyIndices(queueFamilyIndices);
        }
        else
        {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
        }

        createInfo.preTransform = details.Capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; // Here we can choose to use window blending
        createInfo.presentMode = presentMode;
        createInfo.clipped = vk::True;

        createInfo.oldSwapchain = nullptr;

        SwapchainKHR = device.createSwapchainKHR(createInfo);

        if (!SwapchainKHR)
        {
            throw std::runtime_error("Could not create swapchain!");
        }

        SwapchainImages = device.getSwapchainImagesKHR(SwapchainKHR);
        SwapchainImageFormat = surfaceFormat.format;
        SwapchainExtent = extent;

        // Create image views
        SwapchainImageViews.resize(SwapchainImages.size());
        for (size_t i = 0; i < SwapchainImages.size(); i++)
        {
            vk::ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo.image = SwapchainImages[i];
            imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
            imageViewCreateInfo.format = SwapchainImageFormat;
            imageViewCreateInfo.setComponents(vk::ComponentMapping()); // All component swizzle identity
            imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            SwapchainImageViews[i] = device.createImageView(imageViewCreateInfo);
        }
    }

    const vk::SwapchainKHR &Swapchain::GetSwapchainKHR() const
    {
        return SwapchainKHR;
    }

    vk::Extent2D Swapchain::GetSwapchainExtent() const
    {
        return SwapchainExtent;
    }

    vk::Image Swapchain::GetImage(uint32_t imageIndex)
    {
        if (imageIndex >= SwapchainImageViews.size())
        {
            throw std::runtime_error("Cannot get an image index larger than the size of SwapchainImages");
        }
        return SwapchainImages[imageIndex];
    }

    vk::ImageView Swapchain::GetImageView(uint32_t imageIndex)
    {
        if (imageIndex >= SwapchainImageViews.size())
        {
            throw std::runtime_error("Cannot get an image index larger than the size of SwapchainImageViews");
        }
        return SwapchainImageViews[imageIndex];
    }

    uint32_t Swapchain::GetImageCount() const
    {
        return ImageCount;
    }

    vk::Format Swapchain::GetImageFormat() const
    {
        return SwapchainImageFormat;
    }
} // Spinner