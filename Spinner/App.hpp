#ifndef SPINNER_APP_HPP
#define SPINNER_APP_HPP

#include <string>
#include "VulkanInstance.hpp"
#include "Graphics.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "Components/MeshComponent.hpp"
#include "MeshData/StaticMeshVertex.hpp"

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
        std::shared_ptr<Spinner::Scene> Scene;

        std::shared_ptr<Spinner::Components::MeshComponent> MeshComponent;

    public:
        void Run();

    protected:
        virtual void AppInit();
        virtual void DrawScene(CommandBuffer::Pointer &commandBuffer, uint32_t currentFrame, uint32_t imageIndex);
        void AppCleanup();
    };

} // Spinner

#endif //SPINNER_APP_HPP
