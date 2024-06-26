#include "App.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include "MeshData/StaticMeshVertex.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/CameraComponent.hpp"

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

            while (!MainWindow->ShouldClose())
            {
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

        auto suzanneObject = Scene::LoadModel("suzanne.glb");

        if (!Scene->AddObjectToScene(suzanneObject))
        {
            throw std::runtime_error("Suzanne was unable to be loaded");
        }

        auto cameraObject = SceneObject::Create("Main Camera");
        auto camera = cameraObject->AddComponent<Components::CameraComponent>();
        camera->SetActiveCamera();
        cameraObject->SetLocalPosition({0, 0, 5});
        cameraObject->SetLocalRotation({0, 0, 0, 1});
        cameraObject->SetLocalMatrix(glm::inverse(glm::lookAt(cameraObject->GetLocalPosition(), {0, 0, 0}, {0, 1, 0})));

        Scene->AddObjectToScene(cameraObject);
        CameraObject = cameraObject;
    }

    void App::AppCleanup()
    {
        ImGuiInstance.reset();
        DepthImage.reset();

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

        vk::ClearValue colorClearValue;
        colorClearValue.color = {0.4f, 0.58f, 0.93f, 1.0f}; // Cornflower Blue

        {
            vk::RenderingAttachmentInfo colorAttachmentInfo;
            colorAttachmentInfo.imageView = swapchainImageView;
            colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
            colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            colorAttachmentInfo.clearValue = colorClearValue;

            vk::RenderingAttachmentInfo depthAttachmentInfo;
            depthAttachmentInfo.imageView = depthImageView;
            depthAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
            depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            depthAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0u}; // Depth clear value

            vk::Rect2D renderArea({0, 0}, Graphics::GetSwapchainExtent());

            vk::RenderingInfo renderingInfo;
            renderingInfo.renderArea = renderArea;
            renderingInfo.layerCount = 1;
            renderingInfo.setColorAttachments(colorAttachmentInfo);
            renderingInfo.pDepthAttachment = &depthAttachmentInfo;

            commandBuffer->BeginRendering(renderingInfo, Graphics::GetSwapchainExtent());

            Scene->Draw(commandBuffer);

            commandBuffer->EndRendering();
        }

        // ImGui
        {
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

            ImGuiInstance::EndFrame(commandBuffer->VkCommandBuffer);

            commandBuffer->EndRendering();
        }

        // Transition color image for presentation
        commandBuffer->InsertImageMemoryBarrier(swapchainImage, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eNone, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    }

    void App::AppUpdate()
    {
        ImGuiInstance::StartFrame();

        Scene->Update(Graphics->CurrentFrame);

        if (!CameraObject.expired())
        {
            auto cameraObject = CameraObject.lock();
            auto cameraComponentOptional = cameraObject->GetFirstComponent<Components::CameraComponent>();

            if (cameraComponentOptional.has_value() && ImGui::Begin("Camera Properties"))
            {
                auto cameraComponent = cameraComponentOptional.value();
                glm::vec3 cameraPosition = cameraObject->GetLocalPosition();
                if (ImGui::DragFloat3("Position", &cameraPosition.x, 0.05f))
                {
                    cameraObject->SetLocalPosition(cameraPosition);
                }

                glm::quat cameraRotation = cameraObject->GetLocalRotation();
                if (ImGui::DragFloat4("Rotation", &cameraRotation.x, 0.01f))
                {
                    cameraObject->SetLocalRotation(cameraRotation);
                }

                float fov = cameraComponent->GetFOV();
                if (ImGui::DragFloat("FOV", &fov, 0.1f, 0.5f, 175.0f))
                {
                    cameraComponent->SetFOV(fov);
                }

                ImGui::End();
            }
        }
    }

    void App::RecreateDepthImage()
    {
        DepthImage.reset();

        DepthImage = Image::CreateImage(Graphics::GetSwapchainExtent(), Graphics->FindDepthFormat(), vk::ImageUsageFlagBits::eDepthStencilAttachment);
        DepthImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth);
    }
} // Spinner