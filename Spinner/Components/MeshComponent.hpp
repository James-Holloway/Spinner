#ifndef SPINNER_MESHCOMPONENT_HPP
#define SPINNER_MESHCOMPONENT_HPP

#include <bitset>
#include "Component.hpp"
#include "../Shader.hpp"
#include "../MeshBuffer.hpp"
#include "../Constants.hpp"
#include "../Material.hpp"
#include "../VulkanInstance.hpp"

namespace Spinner
{
    class CommandBuffer;

    class Scene;

    class Lighting;

    namespace Components
    {
        class MeshComponent : public Component
        {
        public:
            using ConstantBufferType = MeshConstants;

            MeshComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            Spinner::ShaderInstance::Pointer VertexShaderInstance;
            Spinner::ShaderInstance::Pointer FragmentShaderInstance;
            Spinner::MeshBuffer::Pointer MeshBuffer;
            Spinner::Material::Pointer Material = nullptr;
            Spinner::Buffer::Pointer ConstantBuffer;
            ConstantBufferType LocalConstantBuffer{};
            std::bitset<MAX_FRAMES_IN_FLIGHT> ConstantBindingsDirty;

        protected:
            void SetConstantBindingsDirty();

        public:
            Spinner::ShaderInstance::Pointer GetVertexShaderInstance();
            Spinner::ShaderInstance::Pointer GetFragmentShaderInstance();
            Spinner::MeshBuffer::Pointer GetMeshBuffer();
            Spinner::Shader::Pointer GetVertexShader();
            Spinner::Shader::Pointer GetFragmentShader();

            void SetVertexShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetFragmentShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshBuffer);

            [[nodiscard]] Spinner::Material::Pointer GetMaterial();
            void SetMaterial(const Spinner::Material::Pointer &material);

            void Update(const std::shared_ptr<Lighting> &lighting, uint32_t currentFrame);
            void Draw(const std::shared_ptr<CommandBuffer> &commandBuffer);

            void PopulateFromShaders(const Spinner::Shader::Pointer &vertexShader, const Spinner::Shader::Pointer &fragmentShader, const Spinner::DescriptorPool::Pointer &descriptorPool);

            void UpdateConstantBuffer(const ConstantBufferType &constants);
            ConstantBufferType GetMeshConstants();

            void BindShaders(std::shared_ptr<CommandBuffer> &commandBuffer);

            void RenderDebugUI();
        };

        template<>
        inline constexpr ComponentId GetComponentId<MeshComponent>()
        {
            return 1;
        }

        template<>
        inline const char *GetComponentName<MeshComponent>()
        {
            return "MeshComponent";
        }

        template<>
        inline void RenderDebugUI<MeshComponent>(MeshComponent *component)
        {
            component->RenderDebugUI();
        }
    }
}

#endif //SPINNER_MESHCOMPONENT_HPP
