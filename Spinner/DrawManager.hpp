#ifndef DRAWMANAGER_HPP
#define DRAWMANAGER_HPP

#include "SceneObject.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/MeshComponent.hpp"

namespace Spinner
{
    class DrawManager final
    {
    public:
        using Pointer = std::shared_ptr<DrawManager>;

        DrawManager();
        ~DrawManager();

    protected:
        std::weak_ptr<Spinner::Scene> Scene;

        Buffer::Pointer SceneBuffer;
        SceneConstants LocalSceneBuffer{};

    public:
        void SetScene(const std::shared_ptr<Spinner::Scene> &scene);
        void Update(const Components::ComponentPtr<Components::CameraComponent> &cameraComponent);
        void Render(CommandBuffer::Pointer &commandBuffer);
    };
}


#endif //DRAWMANAGER_HPP
