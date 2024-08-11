#include "ImGuiInstance.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Spinner
{
    ImGuiInstance::ImGuiInstance(const std::shared_ptr<Spinner::Graphics> &graphics)
    {
        DescriptorPool = DescriptorPool::CreateDefault();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        SwapchainSupportDetails details{Graphics::GetPhysicalDevice(), graphics->GetMainSurface()};
        auto imageCount = graphics->GetSwapchainRawPointer()->GetImageCount();
        auto swapchainImageFormat = (VkFormat) graphics->GetSwapchainRawPointer()->GetImageFormat();

        ImGui_ImplGlfw_InitForVulkan(graphics->GetMainWindow()->GetWindow(), true);
        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = VulkanInstance::GetInstance();
        initInfo.PhysicalDevice = Graphics::GetPhysicalDevice();
        initInfo.Device = Graphics::GetDevice();
        initInfo.QueueFamily = graphics->GetGraphicsQueueFamily();
        initInfo.Queue = graphics->GetGraphicsQueue();
        initInfo.DescriptorPool = DescriptorPool->VkDescriptorPool;
        initInfo.MinImageCount = imageCount;
        initInfo.ImageCount = imageCount;
        initInfo.MSAASamples = (VkSampleCountFlagBits) vk::SampleCountFlagBits::e1;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
        initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;

        ImGui_ImplVulkan_Init(&initInfo);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    ImGuiInstance::~ImGuiInstance()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        DescriptorPool.reset();
    }

    void ImGuiInstance::StartFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiInstance::EndFrame(const CommandBuffer::Pointer &commandBuffer, const vk::ImageView &swapchainImageView)
    {
        ImGui::Render();

        vk::ClearValue colorClearValue;
        colorClearValue.color = {0.0f, 0.0f, 0.0f, 1.0f}; // Black

        vk::RenderingAttachmentInfo colorAttachmentInfo;
        colorAttachmentInfo.imageView = swapchainImageView;
        colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
        colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad;
        colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachmentInfo.clearValue = colorClearValue;

        vk::Rect2D renderArea({0, 0}, Graphics::GetSwapchainExtent());

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        renderingInfo.setColorAttachments(colorAttachmentInfo);

        renderingInfo.pDepthAttachment = nullptr;
        commandBuffer->BeginRendering(renderingInfo, Graphics::GetSwapchainExtent());

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->VkCommandBuffer);

        commandBuffer->EndRendering();
    }
} // Spinner