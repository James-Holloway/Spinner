#ifndef SPINNER_APP_HPP
#define SPINNER_APP_HPP

#include <string>
#include "VulkanInstance.hpp"
#include "Graphics.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "ImGuiInstance.hpp"
#include "Image.hpp"
#include "VulkanUtilities.hpp"
#include "Lighting.hpp"

namespace Spinner
{
    class App : public Object
    {
    public:
        explicit App(const std::string &appName, unsigned int appVersion = vk::makeApiVersion(0, 1, 0, 0));
        ~App() override;

    protected:
        std::shared_ptr<Spinner::Window> MainWindow;
        std::shared_ptr<Spinner::Graphics> Graphics;
        Spinner::Scene::Pointer Scene;
        Spinner::ImGuiInstance::Pointer ImGuiInstance;
        Spinner::Lighting::Pointer Lighting;

        Spinner::Image::Pointer DepthImage;
        bool ViewDebugUI = true;
        double FrameDeltaTime = 0.0f;

        float MouseSensitivity = 1.0f;
        int BaseMovementSpeed = 0;
        
        constexpr static float BaseMovementSpeedModifier = 1.0f / 16.0f;

    public:
        void Run();

    protected:
        virtual void AppInit();
        virtual void AppRender(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex);
        virtual void AppCleanup();
        virtual void AppUpdate();
        virtual void AppImGui();
        void RecreateDepthImage();
    };

} // Spinner

#endif //SPINNER_APP_HPP
