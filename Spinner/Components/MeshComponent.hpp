#ifndef SPINNER_MESHCOMPONENT_HPP
#define SPINNER_MESHCOMPONENT_HPP

#include "Component.hpp"
#include "../Shader.hpp"
#include "../MeshBuffer.hpp"
#include "../Constants.hpp"

namespace Spinner
{
    class CommandBuffer;
    class Scene;

    namespace Components
    {
        class MeshComponent : public Component
        {
        public:
            using ConstantBufferType = MeshConstants;

            explicit MeshComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            Spinner::ShaderInstance::Pointer VertexShaderInstance;
            Spinner::ShaderInstance::Pointer FragmentShaderInstance;
            Spinner::MeshBuffer::Pointer MeshBuffer;
            Spinner::Buffer::Pointer ConstantBuffer;
            ConstantBufferType LocalConstantBuffer;
            int ConstantsDirtyCount = MAX_FRAMES_IN_FLIGHT;

        public:
            Spinner::ShaderInstance::Pointer GetVertexShaderInstance();
            Spinner::ShaderInstance::Pointer GetFragmentShaderInstance();
            Spinner::MeshBuffer::Pointer GetMeshBuffer();

            void SetVertexShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetFragmentShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshBuffer);

            void Update(const std::shared_ptr<Scene>& scene, uint32_t currentFrame);
            void Draw(const std::shared_ptr<CommandBuffer> &commandBuffer);

            void PopulateFromShaders(const Spinner::Shader::Pointer &vertexShader, const Spinner::Shader::Pointer &fragmentShader, const Spinner::DescriptorPool::Pointer &descriptorPool);

            void UpdateConstantBuffer(const ConstantBufferType &constants);
            ConstantBufferType GetMeshConstants();
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
    }
}

#endif //SPINNER_MESHCOMPONENT_HPP
