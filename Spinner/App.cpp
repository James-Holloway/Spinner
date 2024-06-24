#include "App.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Spinner
{
    App::App(const std::string &appName, unsigned int appVersion)
    {
        glfwInit();

#ifdef NDEBUG
        bool debug = false;
#else
        bool debug = true;
#endif
        const unsigned int vulkanVersion = vk::ApiVersion13;

        VulkanInstance::CreateInstance(appName, vulkanVersion, appVersion, debug, {}, {});

        MainWindow = std::make_shared<Spinner::Window>();
        MainWindow->Create(1600, 900, appName);

        std::vector<const char *> deviceExtensions{};
        Graphics = std::make_shared<Spinner::Graphics>(MainWindow, deviceExtensions);

        SetSingleCallback(Graphics->RecordGraphicsCommandCallback, [this](CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex) -> void
        {
            DrawScene(commandBuffer, currentFrame, imageIndex);
        });
    }

    App::~App()
    {
        AppCleanup();

        Scene.reset();
        Graphics.reset();

        if (MainWindow->IsValid())
        {
            MainWindow->Destroy();
            MainWindow.reset();
        }

        if (VulkanInstance::HasInstanceBeenCreated())
        {
            VulkanInstance::DestroyInstance();
        }
        glfwTerminate();
    }

    void App::Run()
    {

        try
        {
            AppInit();

            while (!MainWindow->ShouldClose())
            {
                Window::PollEvents();

                Graphics->DrawFrame();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception occurred: " << e.what() << std::endl;
        }
    }

    void App::AppInit()
    {
        // Scene
        Scene = std::make_shared<Spinner::Scene>("Main Scene");

        // Descriptor Pool
        auto descriptorPool = DescriptorPool::CreateDefault();
        Scene->SetDescriptorPool(descriptorPool);

        // Descriptor Set Layout (empty)
        auto descriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout({}, {});

        // Shader creation
        ShaderCreateInfo vertexShaderCreateInfo;
        vertexShaderCreateInfo.ShaderStage = vk::ShaderStageFlagBits::eVertex;
        vertexShaderCreateInfo.ShaderName = "test";
        vertexShaderCreateInfo.NextStage = vk::ShaderStageFlagBits::eFragment;
        vertexShaderCreateInfo.DescriptorSetLayout = descriptorSetLayout;

        ShaderCreateInfo fragmentShaderCreateInfo;
        fragmentShaderCreateInfo.ShaderStage = vk::ShaderStageFlagBits::eFragment;
        fragmentShaderCreateInfo.ShaderName = "test";
        fragmentShaderCreateInfo.NextStage = {};
        fragmentShaderCreateInfo.DescriptorSetLayout = descriptorSetLayout;

        auto shaders = Shader::CreateLinkedShaders({vertexShaderCreateInfo, fragmentShaderCreateInfo});

        // Create test triangle
        auto triangle = MeshData::StaticMeshVertex::CreateTestTriangle();
        // Create a testing mesh component
        MeshComponent = std::make_shared<Components::MeshComponent>();
        MeshComponent->SetMeshBuffer(triangle);
        // Create and set shader instances
        MeshComponent->PopulateFromShaders(shaders[0], shaders[1], descriptorPool);
    }

    void App::AppCleanup()
    {
        MeshComponent.reset();
    }

    void App::DrawScene(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
    {
        auto swapchainImage = Graphics->Swapchain->GetImage(imageIndex);
        auto swapchainImageView = Graphics->Swapchain->GetImageView(imageIndex);

        // Transition color + depth (todo) for drawing
        commandBuffer->InsertImageMemoryBarrier(swapchainImage, vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        vk::ClearValue colorClearValue;
        colorClearValue.color = {0.4f, 0.58f, 0.93f, 1.0f}; // Cornflower Blue

        vk::RenderingAttachmentInfo colorAttachmentInfo;
        colorAttachmentInfo.imageView = swapchainImageView;
        colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
        colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachmentInfo.clearValue = colorClearValue;

        vk::Rect2D renderArea({0, 0}, Graphics::GetSwapchainExtent());

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        renderingInfo.setColorAttachments(colorAttachmentInfo);

        commandBuffer->BeginRendering(renderingInfo, Graphics::GetSwapchainExtent());

        MeshComponent->Draw(commandBuffer);

        commandBuffer->EndRendering();

        // Transition color image for presentation
        commandBuffer->InsertImageMemoryBarrier(swapchainImage, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    }
} // Spinner