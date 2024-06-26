#ifndef SPINNER_GRAPHICS_HPP
#define SPINNER_GRAPHICS_HPP

#include <memory>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.hpp"
#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapchainSupportDetails.hpp"
#include "Swapchain.hpp"
#include "Callback.hpp"
#include "Object.hpp"
#include "CommandBuffer.hpp"

namespace Spinner
{

    class Graphics : public Object
    {
        friend class App;

    public:
        explicit Graphics(const std::shared_ptr<Spinner::Window> &mainWindow, std::vector<const char *> deviceExtensions);
        ~Graphics() override;

    protected:
        vk::PhysicalDevice PhysicalDevice;
        vk::Device Device;
        vma::Allocator Allocator;
        std::shared_ptr<Spinner::Window> MainWindow;

        vk::Queue GraphicsQueue;
        vk::Queue PresentQueue;

        uint32_t PresentQueueFamilyIndex = ~0u;
        uint32_t GraphicsQueueFamilyIndex = ~0u;
        uint32_t PresentQueueIndex = ~0u;
        uint32_t GraphicsQueueIndex = ~0u;

        std::unique_ptr<Spinner::Swapchain> Swapchain;

        bool VSync = false;

        uint32_t CurrentFrame = 0;
        std::array<vk::Fence, MAX_FRAMES_IN_FLIGHT> InFlightGraphicsFences;
        std::array<vk::Semaphore, MAX_FRAMES_IN_FLIGHT> ImageAvailableSemaphores;
        std::array<vk::Semaphore, MAX_FRAMES_IN_FLIGHT> RenderFinishedSemaphores;

        vk::CommandPool GraphicsCommandPool;
        std::vector<CommandBuffer::Pointer> FrameGraphicsCommandBuffers;

    public:
        Callback<int, int> ResizedCallback;
        CallbackSingle<CommandBuffer::Pointer &, uint32_t, uint32_t> RecordGraphicsCommandCallback;

    protected:
        void PickPhysicalDevice(const std::vector<const char *> &deviceExtensions);
        int RateDeviceSuitability(const vk::PhysicalDevice &device, const std::vector<const char *> &deviceExtensions);
        [[nodiscard]] static bool CheckDeviceExtensionSupport(std::vector<const char *> extensions, const vk::PhysicalDevice &physicalDevice);
        void CreateLogicalDevice(const std::vector<const char *> &deviceExtensions);
        void RecreateSwapchain();
        void CreateVmaAllocator();
        void CreateCommandPool();
        void CreateFrameCommandBuffers();
        void CreateSyncObjects();
        void RecordGraphicsCommandBuffer(CommandBuffer::Pointer &commandBuffer, uint32_t imageIndex);

    public:
        static QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface);
        void DrawFrame();

        [[nodiscard]] static const vk::PhysicalDevice &GetPhysicalDevice();
        [[nodiscard]] static const vk::Device &GetDevice();
        [[nodiscard]] static const vma::Allocator &GetAllocator();
        [[nodiscard]] static vk::Extent2D GetSwapchainExtent();

        [[nodiscard]] static std::vector<CommandBuffer::Pointer> CreateCommandBuffers(uint32_t count, bool secondary);
        [[nodiscard]] static CommandBuffer::Pointer BeginSingleTimeCommands();
        static void EndSingleTimeCommands(const CommandBuffer::Pointer& commandBuffer);

        static uint32_t GetCurrentFrame();
    };

} // Spinner

#endif //SPINNER_GRAPHICS_HPP
