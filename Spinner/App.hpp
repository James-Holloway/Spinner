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
        double FrameDeltaTime = 0.0f;

    public:
        void Run();

    protected:
        virtual void AppInit() = 0;
        virtual void AppRender(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex) = 0;
        virtual void AppCleanup() = 0;
        virtual void AppUpdate() = 0;
        virtual void AppImGui() = 0;
    };
} // Spinner

#endif //SPINNER_APP_HPP
