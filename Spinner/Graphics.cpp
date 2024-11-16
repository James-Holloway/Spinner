#include "Graphics.hpp"
#include <map>
#include <set>
#include <GLFW/glfw3.h>

namespace Spinner
{
    static std::vector<const char *> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static Graphics *GraphicsInstance;

    Graphics::Graphics(const std::shared_ptr<Spinner::Window> &mainWindow, std::vector<const char *> deviceExtensions) : MainWindow(mainWindow)
    {
        if (GraphicsInstance != nullptr)
        {
            throw std::runtime_error("Cannot create more than one graphics instance at once");
        }

        if (MainWindow == nullptr || !MainWindow->IsValid())
        {
            throw std::runtime_error("Cannot create graphics with an invalid main window");
        }
        MainWindow->CreateSurface();
        Swapchain = std::make_unique<Spinner::Swapchain>();

        deviceExtensions.push_back(vk::KHRSwapchainExtensionName);
        deviceExtensions.push_back(vk::EXTVertexInputDynamicStateExtensionName);
        PickPhysicalDevice(deviceExtensions);
        CreateLogicalDevice(deviceExtensions);
        CreateVmaAllocator();
        CreateCommandPool();

        GraphicsInstance = this;

        CreateFrameCommandBuffers();
        CreateSyncObjects();

        RecreateSwapchain();
    }

    Graphics::~Graphics()
    {
        Device.waitIdle();

        for (auto &commandBuffer : FrameGraphicsCommandBuffers)
        {
            if (commandBuffer != nullptr)
            {
                commandBuffer->Completed();
            }
        }

        // Just in case something is started in a command buffer completion callback
        Device.waitIdle();

        // Sync Objects
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (ImageAvailableSemaphores[i])
            {
                Device.destroySemaphore(ImageAvailableSemaphores[i]);
            }
            if (RenderFinishedSemaphores[i])
            {
                Device.destroySemaphore(RenderFinishedSemaphores[i]);
            }
            if (InFlightGraphicsFences[i])
            {
                Device.destroyFence(InFlightGraphicsFences[i]);
            }
        }

        // Command Pool
        if (GraphicsCommandPool)
        {
            Device.destroyCommandPool(GraphicsCommandPool);
        }

        if (Swapchain)
        {
            Swapchain.reset();
        }

        if (Allocator)
        {
            Allocator.destroy();
        }
        if (Device)
        {
            Device.destroy();
        }
        // PhysicalDevice does not need to be destroyed

        if (MainWindow != nullptr)
        {
            if (MainWindow->GetSurface())
            {
                MainWindow->DestroySurface();
            }
            MainWindow.reset();
        }

        if (GraphicsInstance == this)
        {
            GraphicsInstance = nullptr;
        }
    }

    const vk::PhysicalDevice &Graphics::GetPhysicalDevice()
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot get the physical device from a non-existent Graphics instance");
        }

        return GraphicsInstance->PhysicalDevice;
    }

    const vk::Device &Graphics::GetDevice()
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot get the logical device from a non-existent Graphics instance");
        }

        return GraphicsInstance->Device;
    }

    const vma::Allocator &Graphics::GetAllocator()
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot get the Vma Allocator from a non-existent Graphics instance");
        }

        return GraphicsInstance->Allocator;
    }

    void Graphics::PickPhysicalDevice(const std::vector<const char *> &deviceExtensions)
    {
        std::vector<vk::PhysicalDevice> devices = VulkanInstance::GetInstance().enumeratePhysicalDevices();

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, vk::PhysicalDevice> candidates;

        for (const auto &device : devices)
        {
            int score = RateDeviceSuitability(device, deviceExtensions);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0)
        {
            PhysicalDevice = candidates.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        if (!PhysicalDevice)
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    int Graphics::RateDeviceSuitability(const vk::PhysicalDevice &device, const std::vector<const char *> &deviceExtensions)
    {
        vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
        auto physicalFeaturesChain = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceShaderObjectFeaturesEXT, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();

        auto &deviceFeatures = physicalFeaturesChain.get<vk::PhysicalDeviceFeatures2>().features;
        auto &vulkan11Features = physicalFeaturesChain.get<vk::PhysicalDeviceVulkan11Features>();
        auto &vulkan12Features = physicalFeaturesChain.get<vk::PhysicalDeviceVulkan12Features>();
        auto &vulkan13Features = physicalFeaturesChain.get<vk::PhysicalDeviceVulkan13Features>();
        auto &shaderObjectFeatures = physicalFeaturesChain.get<vk::PhysicalDeviceShaderObjectFeaturesEXT>();
        auto &extendedDynamicStateFeatures = physicalFeaturesChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        auto &vertexInputDynamicStateFeatures = physicalFeaturesChain.get<vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();

        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);

        // Application can't function without geometry shaders, sampler anisotropy, dynamic rendering & states, shader objects and synchronization2
        if (!deviceFeatures.geometryShader || !deviceFeatures.samplerAnisotropy || !vulkan13Features.dynamicRendering || !shaderObjectFeatures.shaderObject || !extendedDynamicStateFeatures.extendedDynamicState || !vertexInputDynamicStateFeatures.vertexInputDynamicState || !vulkan13Features.synchronization2)
        {
            return 0;
        }

        // Application also requires bindless descriptor features & runtime descriptor array
        if (!vulkan12Features.descriptorBindingPartiallyBound || !vulkan12Features.shaderSampledImageArrayNonUniformIndexing || !vulkan12Features.shaderUniformBufferArrayNonUniformIndexing || !vulkan12Features.shaderStorageBufferArrayNonUniformIndexing || !vulkan12Features.descriptorBindingSampledImageUpdateAfterBind || !vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind || !vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind || !vulkan12Features.runtimeDescriptorArray)
        {
            return 0;
        }

        // This application cannot function without a graphics, present and compute queue
        QueueFamilyIndices indices = FindQueueFamilies(device, MainWindow->GetSurface());
        if (!indices.IsComplete())
        {
            return 0;
        }

        // We require all the device extensions to be supported
        if (!CheckDeviceExtensionSupport(deviceExtensions, device))
        {
            return 0;
        }

        SwapchainSupportDetails swapChainSupport{device, MainWindow->GetSurface()};
        // Swapchain inadequate
        if (swapChainSupport.Formats.empty() || swapChainSupport.PresentModes.empty())
        {
            return 0;
        }

        return score;
    }

    QueueFamilyIndices Graphics::FindQueueFamilies(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface)
    {
        QueueFamilyIndices indices;

        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.GraphicsFamily = i;
                indices.GraphicsFamilyQueueCount = queueFamily.queueCount;
            }

            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
            {
                indices.ComputeFamily = i;
                indices.ComputeFamilyQueueCount = queueFamily.queueCount;
            }

            if (physicalDevice.getSurfaceSupportKHR(i, surface))
            {
                indices.PresentFamily = i;
                indices.PresentFamilyQueueCount = queueFamily.queueCount;
            }

            if (indices.IsComplete())
                break;

            i++;
        }

        return indices;
    }

    bool Graphics::CheckDeviceExtensionSupport(std::vector<const char *> extensions, const vk::PhysicalDevice &physicalDevice)
    {
        std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        // Remove items from this set when they are found in available extensions
        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void Graphics::CreateLogicalDevice(const std::vector<const char *> &deviceExtensions)
    {
        QueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice, MainWindow->GetSurface());

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        float queuePriority = 1.0f;
        std::vector<float> queuePriorities = {queuePriority, queuePriority};
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.setQueuePriorities(queuePriorities);
            queueCreateInfo.queueCount = 1;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::vector<const char *> extensionsToEnable(deviceExtensions.begin(), deviceExtensions.end());
        extensionsToEnable.push_back(vk::EXTShaderObjectExtensionName);
        extensionsToEnable.push_back(vk::EXTVertexInputDynamicStateExtensionName);

        vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceShaderObjectFeaturesEXT, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT> chain;

        auto &deviceFeatures = chain.get<vk::PhysicalDeviceFeatures2>().features;
        deviceFeatures.samplerAnisotropy = true;

        auto &vulkan11Features = chain.get<vk::PhysicalDeviceVulkan11Features>();

        auto &vulkan12Features = chain.get<vk::PhysicalDeviceVulkan12Features>();
        vulkan12Features.descriptorBindingPartiallyBound = true;
        vulkan12Features.shaderSampledImageArrayNonUniformIndexing = true;
        vulkan12Features.shaderUniformBufferArrayNonUniformIndexing = true;
        vulkan12Features.shaderStorageBufferArrayNonUniformIndexing = true;
        vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = true;
        vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind = true;
        vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = true;
        vulkan12Features.runtimeDescriptorArray = true;

        auto &vulkan13Features = chain.get<vk::PhysicalDeviceVulkan13Features>();
        vulkan13Features.dynamicRendering = true;
        vulkan13Features.synchronization2 = true;

        auto &shaderObjectFeatures = chain.get<vk::PhysicalDeviceShaderObjectFeaturesEXT>();
        shaderObjectFeatures.shaderObject = true;

        auto &extendedDynamicStateFeatures = chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        extendedDynamicStateFeatures.extendedDynamicState = true;

        auto &vertexInputDynamicStateFeatures = chain.get<vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
        vertexInputDynamicStateFeatures.vertexInputDynamicState = true;

        auto &deviceCreateInfo = chain.get<vk::DeviceCreateInfo>();
        deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
        deviceCreateInfo.setPEnabledFeatures(nullptr);
        deviceCreateInfo.setPEnabledExtensionNames(extensionsToEnable);

        // Actual creation of the device happens here
        Device = PhysicalDevice.createDevice(chain.get<vk::DeviceCreateInfo>());

        if (!Device)
        {
            throw std::runtime_error("Failed to create logical device!");
        }

        VulkanInstance::ReinitializeDispatcher(Device);

        GraphicsQueueIndex = 0;
        PresentQueueIndex = 0;

        GraphicsQueue = Device.getQueue(indices.GraphicsFamily.value(), GraphicsQueueIndex);
        PresentQueue = Device.getQueue(indices.PresentFamily.value(), PresentQueueIndex);

        PresentQueueFamilyIndex = indices.PresentFamily.value();
        GraphicsQueueFamilyIndex = indices.GraphicsFamily.value();
    }

    void Graphics::RecreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(MainWindow->GetWindow(), &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(MainWindow->GetWindow(), &width, &height);
            glfwWaitEvents();
        }

        Device.waitIdle();

        Swapchain->Recreate(Device, PhysicalDevice, MainWindow->GetSurface(), width, height, VSync);

        ResizedCallback.Run(width, height);
    }

    void Graphics::CreateVmaAllocator()
    {
        vma::AllocatorCreateInfo createInfo;
        createInfo.vulkanApiVersion = VulkanInstance::GetVulkanVersion();
        createInfo.flags = {};
        createInfo.instance = VulkanInstance::GetInstance();
        createInfo.physicalDevice = PhysicalDevice;
        createInfo.device = Device;

        Allocator = vma::createAllocator(createInfo);
    }

    void Graphics::CreateCommandPool()
    {
        vk::CommandPoolCreateInfo graphicsPoolCreateInfo;
        graphicsPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        graphicsPoolCreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;

        GraphicsCommandPool = Device.createCommandPool(graphicsPoolCreateInfo);
    }

    void Graphics::CreateFrameCommandBuffers()
    {
        FrameGraphicsCommandBuffers = CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, false);
    }

    void Graphics::CreateSyncObjects()
    {
        vk::SemaphoreCreateInfo semaphoreCreateInfo;

        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            ImageAvailableSemaphores[i] = Device.createSemaphore(semaphoreCreateInfo);
            RenderFinishedSemaphores[i] = Device.createSemaphore(semaphoreCreateInfo);
            InFlightGraphicsFences[i] = Device.createFence(fenceCreateInfo);
        }
    }

    void Graphics::RecordGraphicsCommandBuffer(CommandBuffer::Pointer &commandBuffer, uint32_t imageIndex)
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = {};
        beginInfo.pInheritanceInfo = nullptr;

        if (commandBuffer->Begin(&beginInfo) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to begin recording graphics command buffer");
        }

        RecordGraphicsCommandCallback.Run(commandBuffer, CurrentFrame, imageIndex);

        commandBuffer->End();
    }

    constexpr unsigned long LongTimeTimeout = 1ul * 1000ul * 1000ul * 1000ul; // 1 seconds (2e9 nanoseconds)

    void Graphics::DrawFrame()
    {
        vk::detail::resultCheck(Device.waitForFences(1, &InFlightGraphicsFences[CurrentFrame], true, LongTimeTimeout), "Failed while waiting for previous frame fence");

        // Acquire next image
        uint32_t imageIndex = 0;
        auto nextImageResult = Device.acquireNextImageKHR(Swapchain->GetSwapchainKHR(), LongTimeTimeout, ImageAvailableSemaphores[CurrentFrame], nullptr, &imageIndex);
        if (nextImageResult == vk::Result::eErrorOutOfDateKHR)
        {
            RecreateSwapchain();
            return;
        }
        else if (nextImageResult != vk::Result::eSuccess && nextImageResult != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        // Reset frame fence
        (void) Device.resetFences(1, &InFlightGraphicsFences[CurrentFrame]);

        // Record command buffers
        CommandBuffer::Pointer &commandBuffer = FrameGraphicsCommandBuffers[CurrentFrame];
        commandBuffer->Completed();
        commandBuffer->Reset();
        RecordGraphicsCommandBuffer(commandBuffer, imageIndex);

        // Submit command buffers
        vk::SubmitInfo submitInfo;
        std::vector<vk::Semaphore> waitSemaphores = {ImageAvailableSemaphores[CurrentFrame]};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setWaitSemaphores(waitSemaphores);
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.setCommandBuffers(commandBuffer->VkCommandBuffer);

        std::vector<vk::Semaphore> signalSemaphores = {RenderFinishedSemaphores[CurrentFrame]};
        submitInfo.setSignalSemaphores(signalSemaphores);

        // Submit to the graphics queue
        GraphicsQueue.submit(submitInfo, InFlightGraphicsFences[CurrentFrame]);

        // Present
        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphores);

        presentInfo.setSwapchains(Swapchain->GetSwapchainKHR());
        presentInfo.pImageIndices = &imageIndex;

        vk::Result presentResult = PresentQueue.presentKHR(&presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || MainWindow->DoesFrameBufferNeedsResize())
        {
            MainWindow->SetFrameBufferNeedsResize(false);
            RecreateSwapchain();
        }
        else if (presentResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present frame!");
        }

        // Change current frame
        CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    CommandBuffer::Pointer Graphics::BeginSingleTimeCommands()
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot begin a single time command from a non-existent Graphics instance");
        }

        CommandBuffer::Pointer commandBuffer = CreateCommandBuffers(1, false)[0];

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer->Begin(beginInfo);

        return commandBuffer;
    }

    void Graphics::EndSingleTimeCommands(const CommandBuffer::Pointer &commandBuffer)
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot end and submit a command buffer from a non-existent Graphics instance");
        }

        commandBuffer->End();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer->VkCommandBuffer);

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        auto &device = GetDevice();
        vk::Fence fence = device.createFence(fenceInfo);
        device.resetFences(fence);

        GraphicsInstance->GraphicsQueue.submit(submitInfo, fence);

        auto result = device.waitForFences(fence, true, LongTimeTimeout);

        device.destroyFence(fence);
        device.freeCommandBuffers(GraphicsInstance->GraphicsCommandPool, commandBuffer->VkCommandBuffer);

        commandBuffer->Completed();

        vk::detail::resultCheck(result, "Single time command failed");
    }

    std::vector<CommandBuffer::Pointer> Graphics::CreateCommandBuffers(uint32_t count, bool secondary)
    {
        if (GraphicsInstance == nullptr)
        {
            throw std::runtime_error("Cannot create command buffers from a non-existent Graphics instance");
        }

        vk::CommandBufferAllocateInfo createInfo;
        createInfo.commandPool = GraphicsInstance->GraphicsCommandPool;
        createInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;
        createInfo.commandBufferCount = count;

        auto vkCommandBuffers = GetDevice().allocateCommandBuffers(createInfo);

        std::vector<CommandBuffer::Pointer> commandBuffers{};
        for (auto &vkCommandBuffer : vkCommandBuffers)
        {
            commandBuffers.push_back(std::make_shared<CommandBuffer>(vkCommandBuffer));
        }

        return commandBuffers;
    }

    vk::Extent2D Graphics::GetSwapchainExtent()
    {
        return GraphicsInstance->Swapchain->GetSwapchainExtent();
    }

    uint32_t Graphics::GetCurrentFrame()
    {
        return GraphicsInstance->CurrentFrame;
    }

    std::shared_ptr<Spinner::Window> Graphics::GetMainWindow()
    {
        return MainWindow;
    }

    vk::Queue Graphics::GetGraphicsQueue()
    {
        return GraphicsQueue;
    }

    uint32_t Graphics::GetGraphicsQueueFamily() const
    {
        return GraphicsQueueFamilyIndex;
    }

    Spinner::Swapchain *Graphics::GetSwapchainRawPointer()
    {
        return Swapchain.get();
    }

    vk::SurfaceKHR Graphics::GetMainSurface()
    {
        return MainWindow->GetSurface();
    }

    uint32_t Graphics::GetGraphicsQueueFamilyIndex()
    {
        return GraphicsInstance->GetGraphicsQueueFamily();
    }

    vk::Format Graphics::FindSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
    {
        for (vk::Format format : candidates)
        {
            vk::FormatProperties props = PhysicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    vk::Format Graphics::FindDepthFormat(bool highQuality)
    {
        if (highQuality)
        {
            return FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16Unorm, vk::Format::eD16UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }
        return FindSupportedFormat({vk::Format::eD24UnormS8Uint, vk::Format::eD16Unorm, vk::Format::eD16UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    Input::Pointer Graphics::GetInput()
    {
        if (std::shared_ptr<Window> window = GraphicsInstance->GetMainWindow())
        {
            return window->GetInput();
        }
        throw std::runtime_error("Graphics' MainWindow was nullptr, cannot get Input");
    }
} // Spinner
