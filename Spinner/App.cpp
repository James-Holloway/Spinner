#include "App.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "ScopedTimer.hpp"

namespace Spinner
{
    App::App(const std::string &appName, unsigned int appVersion)
    {
        glfwInit();

#ifdef NDEBUG
        constexpr bool debug = false;
#else
        constexpr bool debug = true;
#endif
        constexpr unsigned int vulkanVersion = vk::ApiVersion13;

        ScopedTimer::Print = debug;

        ScopedTimer timer("App Creation");

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
            {
                ScopedTimer timer("AppInit");
                AppInit();
            }

            ScopedTimer::Print = false;

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

        // Cleanup
        if (Graphics != nullptr)
        {
            Graphics::GetDevice().waitIdle();
        }

        AppCleanup();
    }
} // Spinner
