#include "App.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include "MeshData/StaticMeshVertex.hpp"
#include "Components/Components.hpp"
#include "SceneDescriptors.hpp"

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
        if (Graphics != nullptr)
        {
            Graphics::GetDevice().waitIdle();
        }

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

            auto input = Graphics::GetInput();
            while (!MainWindow->ShouldClose())
            {
                input->ResetFrameKeyState();
                Window::PollEvents();

                AppUpdate();

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
        Texture::CreateDefaultTextures();

        RecreateDepthImage();

        RegisterCallback(Graphics->ResizedCallback, [this](int width, int height) -> void
        {
            RecreateDepthImage();
        });

        // ImGui
        ImGuiInstance = std::make_shared<Spinner::ImGuiInstance>(Graphics);

        // Scene
        Scene = std::make_shared<Spinner::Scene>("Main Scene");

        MeshData::StaticMeshVertex::CreateShaders();

        auto testObject = Scene::LoadModel("sponza lit.glb");

        if (!Scene->AddObjectToScene(testObject))
        {
            throw std::runtime_error("Model was unable to be loaded");
        }

        auto cameraObject = SceneObject::Create("Main Camera");
        MainCamera = cameraObject->AddComponent<Components::CameraComponent>();
        MainCamera->CreateRenderTarget({512, 512}, true, true);
        cameraObject->SetLocalPosition({0, 1, 1.5f});
        cameraObject->SetLocalRotation({1, 0, 0, 0});
        cameraObject->SetLocalEulerRotation({0, 180, 0});
        Scene->AddObjectToScene(cameraObject);

        // auto lightObject = SceneObject::Create("Point light");
        // auto light = lightObject->AddComponent<Components::LightComponent>();
        // light->SetLightType(LightType::Point);
        // lightObject->SetLocalPosition({0, 5, 0});
        // Scene->AddObjectToScene(lightObject);

        MainDrawManager = DrawManager::Create();
    }

    void App::AppCleanup()
    {
        ImGuiInstance.reset();
        DepthImage.reset();

        Texture::ReleaseDefaultTextures();

        MeshData::StaticMeshVertex::DestroyShaders();
    }

    void App::DrawScene(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
    {
        if (DepthImage == nullptr)
        {
            throw std::runtime_error("Cannot draw scene without DepthImage existing");
        }

        auto swapchainImage = Graphics->Swapchain->GetImage(imageIndex);
        auto swapchainImageView = Graphics->Swapchain->GetImageView(imageIndex);
        auto depthImageView = DepthImage->GetMainImageView();

        vk::ImageAspectFlags depthBarrierAspect = vk::ImageAspectFlagBits::eDepth;
        vk::ImageLayout depthImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        if (VkFormatHasStencilComponent(DepthImage->GetFormat()))
        {
            depthBarrierAspect |= vk::ImageAspectFlagBits::eStencil;
            depthImageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        // Transition color + depth (if necessary)
        commandBuffer->InsertImageMemoryBarrier(swapchainImage, vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        commandBuffer->TransitionImageLayout(DepthImage, depthImageLayout, depthBarrierAspect, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::ImageSubresourceRange(depthBarrierAspect, 0, 1, 0, 1));

        // Track depth image
        commandBuffer->TrackObject(DepthImage);

        // Main rendering
        {
            // vk::ClearColorValue colorClearValue = {0.4f, 0.58f, 0.93f, 1.0f}; // Cornflower Blue
            // commandBuffer->BeginRendering(swapchainImageView, depthImageView, {{0, 0}, Graphics::GetSwapchainExtent()}, colorClearValue);

            MainDrawManager->Draw(commandBuffer);

            // commandBuffer->EndRendering();
        }

        // ImGui
        ImGuiInstance::EndFrame(commandBuffer, swapchainImageView);

        // Transition color image for presentation
        commandBuffer->InsertImageMemoryBarrier(swapchainImage, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    }

    void App::AppUpdate()
    {
        ImGuiInstance::StartFrame();

        MainDrawManager->ResetAndUpdate(MainCamera.GetRaw(), Scene);

        auto extentUint = Graphics::GetSwapchainExtent();
        auto extentImGui = ImVec2(static_cast<float>(extentUint.width), static_cast<float>(extentUint.height));

        if (ViewDebugUI)
        {
            if (ImGui::Begin("Hierarchy", &ViewDebugUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
            {
                ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                ImGui::SetWindowSize(ImVec2(350.0f, extentImGui.y));

                Scene->RenderHierarchy();

                ImGui::End();
            }

            if (ImGui::Begin("Properties", &ViewDebugUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
            {
                ImGui::SetWindowPos(ImVec2(extentImGui.x - 350.0f, 0), ImGuiCond_Always);
                ImGui::SetWindowSize(ImVec2(350.0f, extentImGui.y));

                Scene->RenderSelectedProperties();

                ImGui::End();
            }
        }

        auto input = Graphics::GetInput();
        if (input->GetKeyPressed(Key::F3))
        {
            ViewDebugUI = !ViewDebugUI;
        }
    }

    void App::RecreateDepthImage()
    {
        DepthImage.reset();

        DepthImage = Image::CreateImage(Graphics::GetSwapchainExtent(), Graphics::FindDepthFormat(), vk::ImageUsageFlagBits::eDepthStencilAttachment);
        DepthImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth);
    }
} // Spinner