#ifndef SPINNER_MESHCOMPONENT_HPP
#define SPINNER_MESHCOMPONENT_HPP

#include "Component.hpp"
#include "../Shader.hpp"
#include "../MeshBuffer.hpp"

namespace Spinner
{
    class CommandBuffer;

    namespace Components
    {
        class MeshComponent : public Component, public std::enable_shared_from_this<MeshComponent>
        {
        public:
            MeshComponent() = default;

        protected:
            Spinner::ShaderInstance::Pointer VertexShaderInstance;
            Spinner::ShaderInstance::Pointer FragmentShaderInstance;
            Spinner::MeshBuffer::Pointer MeshBuffer;

        public:
            Spinner::ShaderInstance::Pointer GetVertexShaderInstance();
            Spinner::ShaderInstance::Pointer GetFragmentShaderInstance();
            Spinner::MeshBuffer::Pointer GetMeshBuffer();

            void SetVertexShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetFragmentShaderInstance(Spinner::ShaderInstance::Pointer newShaderInstance);
            void SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshBuffer);

            void Draw(const std::shared_ptr<CommandBuffer> &commandBuffer);

            void PopulateFromShaders(const Spinner::Shader::Pointer &vertexShader, const Spinner::Shader::Pointer &fragmentShader, const Spinner::DescriptorPool::Pointer &descriptorPool);
        };

        template<>
        inline ComponentId GetComponentId<MeshComponent>()
        {
            return 1;
        }
    }
}

#endif //SPINNER_MESHCOMPONENT_HPP
