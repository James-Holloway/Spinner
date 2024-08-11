#ifndef SPINNER_DRAWMANAGER_HPP
#define SPINNER_DRAWMANAGER_HPP

#include <vulkan/vulkan.hpp>
#include <map>
#include "CommandBuffer.hpp"
#include "DescriptorPool.hpp"
#include "Constants.hpp"
#include "Buffer.hpp"

namespace Spinner
{
    namespace Components
    {
        class MeshComponent;

        class CameraComponent;
    }

    class Scene;

    class DrawManager
    {
    public:
        using Pointer = std::shared_ptr<DrawManager>;

        DrawManager();

    protected:
        DescriptorPool::Pointer DescriptorPool;
        Buffer::Pointer SceneConstantsBuffer;
        SceneConstants LocalSceneConstantsBuffer{};
        DescriptorSetLayout::Pointer SceneDescriptorSetLayout;
        std::vector<vk::DescriptorSet> SceneDescriptorSets;

        std::map<std::string, std::vector<Components::MeshComponent*>> MeshComponentShaderMap = {};
        Components::CameraComponent* DrawingCameraComponent;

    protected:
        void DrawPass(CommandBuffer::Pointer &commandBuffer, Components::CameraComponent *cameraComponent, const std::map<std::string, std::vector<Components::MeshComponent *>> &meshComponents, bool transparency);
        void Draw(CommandBuffer::Pointer &commandBuffer, Components::CameraComponent *cameraComponent, const std::map<std::string, std::vector<Components::MeshComponent *>> &meshComponents);

    public:
        // Resets descriptor pool - only use once previous use of this DrawManager has completed
        void ResetAndUpdate(Components::CameraComponent *cameraComponent, const std::shared_ptr<Scene> &scene);
        void Draw(CommandBuffer::Pointer &commandBuffer);

    public:
        static Pointer Create();
    };

} // Spinner

#endif //SPINNER_DRAWMANAGER_HPP
