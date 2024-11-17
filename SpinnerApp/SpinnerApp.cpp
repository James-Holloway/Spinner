#include "SpinnerApp.hpp"
#include "Spinner/MeshData/StaticMeshVertex.hpp"
#include "Spinner/Components/Components.hpp"

using namespace Spinner;

SpinnerApp::SpinnerApp() : App("Spinner")
{
}

void SpinnerApp::AppInit()
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

    // Draw Manager
    for (auto &drawManager : DrawManagers)
    {
        drawManager = std::make_shared<Spinner::DrawManager>();
        drawManager->SetScene(Scene);
    }

    MeshData::StaticMeshVertex::CreateShaders();

    auto testObject = Scene::LoadModel("sponza lit.glb");

    if (!Scene->AddObjectToScene(testObject))
    {
        throw std::runtime_error("Model was unable to be loaded");
    }

    auto cameraObject = SceneObject::Create("Main Camera");
    Camera = cameraObject->AddComponent<Components::CameraComponent>();
    Camera->SetActiveCamera();
    cameraObject->SetLocalPosition({0, 1, 1.5f});
    cameraObject->SetLocalRotation({1, 0, 0, 0});
    cameraObject->SetLocalEulerRotation({0, 180, 0});
    CameraController = cameraObject->AddComponent<Components::CameraControllerComponent>();
    Scene->AddObjectToScene(cameraObject);

    // auto lightObject = SceneObject::Create("Point light");
    // auto light = lightObject->AddComponent<Components::LightComponent>();
    // light->SetLightType(LightType::Point);
    // lightObject->SetLocalPosition({0, 5, 0});
    // Scene->AddObjectToScene(lightObject);
}

void SpinnerApp::AppRender(Spinner::CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
{
    if (DepthImage == nullptr)
    {
        throw std::runtime_error("Cannot draw scene without DepthImage existing");
    }

    // Shadows
    DrawManagers[currentFrame]->RenderShadows(commandBuffer);

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

        DrawManagers[currentFrame]->Render(commandBuffer);

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

void SpinnerApp::AppCleanup()
{
    for (auto &drawManager : DrawManagers)
    {
        drawManager.reset();
    }

    Scene.reset();
    ImGuiInstance.reset();
    DepthImage.reset();

    Texture::ReleaseDefaultTextures();

    MeshData::StaticMeshVertex::DestroyShaders();
}

void SpinnerApp::AppUpdate()
{
    ImGuiInstance::StartFrame();
    AppImGui();

    DrawManagers[Graphics::GetCurrentFrame()]->Update(Camera);

    auto input = Graphics::GetInput();
    if (input->GetKeyPressed(Key::F3))
    {
        ViewDebugUI = !ViewDebugUI;
    }

    CameraController->Update(static_cast<float>(FrameDeltaTime));
}

void SpinnerApp::AppImGui()
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
        }
        ImGui::End();

        if (ImGui::Begin("Properties", &ViewDebugUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            ImGui::SetWindowPos(ImVec2(extentImGui.x - 350.0f, 0), ImGuiCond_Always);
            ImGui::SetWindowSize(ImVec2(350.0f, extentImGui.y));

            Scene->RenderSelectedProperties();
        }

        ImGui::End();
    }
}

void SpinnerApp::RecreateDepthImage()
{
    DepthImage.reset();

    DepthImage = Image::CreateImage(Graphics::GetSwapchainExtent(), Graphics->FindDepthFormat(), vk::ImageUsageFlagBits::eDepthStencilAttachment);
    DepthImage->CreateMainImageView(vk::ImageAspectFlagBits::eDepth);
}
