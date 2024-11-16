#ifndef SPINNER_DRAWMANAGER_HPP
#define SPINNER_DRAWMANAGER_HPP

#include "SceneObject.hpp"
#include "DrawCommand.hpp"
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

        Spinner::DescriptorPool::Pointer DescriptorPool;
        Buffer::Pointer SceneBuffer;
        SceneConstants LocalSceneBuffer{};

        std::multimap<Spinner::Pass, Spinner::DrawCommand::Pointer> DrawCommands;

    protected:
        Spinner::DrawCommand::Pointer CreateDrawCommand(const Spinner::ShaderGroup::Pointer &shaderGroup);

    public:
        void SetScene(const std::shared_ptr<Spinner::Scene> &scene);
        void Update(const Components::ComponentPtr<Components::CameraComponent> &cameraComponent);
        void Render(CommandBuffer::Pointer &commandBuffer);
        void RenderShadows(CommandBuffer::Pointer& commandBuffer) const;
    };
}


#endif
