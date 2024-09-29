#include "App.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include "MeshData/StaticMeshVertex.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/LightComponent.hpp"

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
            AppRender(commandBuffer, currentFrame, imageIndex);
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
            auto lastTime = std::chrono::high_resolution_clock::now();
            while (!MainWindow->ShouldClose())
            {
                input->ResetFrameState();
                Window::PollEvents();

                auto nowTime = std::chrono::high_resolution_clock::now();
                FrameDeltaTime = std::chrono::duration<double, std::chrono::seconds::period>(nowTime - lastTime).count();
                lastTime = nowTime;

                AppUpdate();

                ImGuiInstance::StartFrame();
                AppImGui();

                Graphics->DrawFrame();
            }
        } catch (const std::exception &e)
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
        auto camera = cameraObject->AddComponent<Components::CameraComponent>();
        camera->SetActiveCamera();
        cameraObject->SetLocalPosition({0, 1, 1.5f});
        cameraObject->SetLocalRotation({1, 0, 0, 0});
        cameraObject->SetLocalEulerRotation({0, 180, 0});
        Scene->AddObjectToScene(cameraObject);

        // auto lightObject = SceneObject::Create("Point light");
        // auto light = lightObject->AddComponent<Components::LightComponent>();
        // light->SetLightType(LightType::Point);
        // lightObject->SetLocalPosition({0, 5, 0});
        // Scene->AddObjectToScene(lightObject);
    }

    void App::AppCleanup()
    {
        ImGuiInstance.reset();
        DepthImage.reset();

        Texture::ReleaseDefaultTextures();

        MeshData::StaticMeshVertex::DestroyShaders();
    }

    void App::AppRender(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
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

            Scene->Draw(currentFrame, commandBuffer);

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
        Scene->Update(Graphics->CurrentFrame);

        auto input = Graphics::GetInput();
        if (input->GetKeyPressed(Key::F3))
        {
            ViewDebugUI = !ViewDebugUI;
        }

        if (input->GetKeyDown(Key::MouseRight))
        {
            auto activeCamera = Components::CameraComponent::GetActiveCameraRawPointer();
            if (activeCamera != nullptr)
            {
                glm::vec3 movementDirection{0.0f, 0.0f, 0.0f};
                // Forward / Back
                if (input->GetKeyDown(Key::W))
                {
                    movementDirection.z += 1.0f;
                }
                if (input->GetKeyDown(Key::S))
                {
                    movementDirection.z -= 1.0f;
                }
                // Right / Left
                if (input->GetKeyDown(Key::A))
                {
                    movementDirection.x += 1.0f;
                }
                if (input->GetKeyDown(Key::D))
                {
                    movementDirection.x -= 1.0f;
                }
                // Up / Down
                if (input->GetKeyDown(Key::E))
                {
                    movementDirection.y += 1.0f;
                }
                if (input->GetKeyDown(Key::Q))
                {
                    movementDirection.y -= 1.0f;
                }

                float movementSpeed = static_cast<float>(FrameDeltaTime) * 4.0f;

                if (BaseMovementSpeed < 0)
                {
                    movementSpeed *= 1.0f - (static_cast<float>(1 << -BaseMovementSpeed) * BaseMovementSpeedModifier);
                }
                else if (BaseMovementSpeed > 0)
                {
                    movementSpeed *= 1.0f + (static_cast<float>(1 << BaseMovementSpeed) * BaseMovementSpeedModifier);
                }

                if (input->GetKeyDown(Key::LeftShift))
                {
                    movementSpeed *= 4.0f;
                }
                if (input->GetKeyDown(Key::LeftControl))
                {
                    movementSpeed /= 4.0f;
                }

                if (movementDirection.x != 0.0f || movementDirection.y != 0.0f || movementDirection.z != 0.0f)
                {
                    movementDirection = glm::normalize(movementDirection) * movementSpeed;
                    activeCamera->MoveRelative(movementDirection);
                }

                // Euler rotation is in degrees, so 90 is in degrees
                float cameraRotationSpeed = 45.0f * MouseSensitivity * 0.01f;
                double cameraX = 0.0, cameraY = 0.0;
                input->GetCursorDeltaPosition(cameraX, cameraY);

                if (cameraX != 0.0f || cameraY != 0.0f)
                {
                    const glm::vec3 cameraRotation{cameraRotationSpeed * cameraY, cameraRotationSpeed * -cameraX, 0.0f};
                    activeCamera->RotateEuler(cameraRotation);
                }

                // Scroll in and out to change movement speed
                const auto scrollY = static_cast<float>(input->GetVerticalScrollDelta());
                if (scrollY < 0.0f)
                {
                    BaseMovementSpeed -= 1;
                }
                else if (scrollY > 0.0f)
                {
                    BaseMovementSpeed += 1;
                }
            }
        }
    }

    void App::AppImGui()
    {
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
    }

    void App::RecreateDepthImage()
    {
        DepthImage.reset();

        DepthImage = Image::CreateImage(Graphics::GetSwapchainExtent(), Graphics->FindDepthFormat(), vk::ImageUsageFlagBits::eDepthStencilAttachment);
        DepthImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth);
    }
} // Spinner
