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
    class DrawCommand;

    namespace Components
    {
        class MeshComponent : public Component
        {
        public:
            using ConstantBufferType = MeshConstants;

            MeshComponent(const std::weak_ptr<Spinner::SceneObject> &sceneObject, int64_t componentIndex);

        protected:
            Spinner::ShaderGroup::Pointer ShaderGroup;
            Spinner::MeshBuffer::Pointer MeshBuffer;
            Spinner::Material::Pointer Material = nullptr;
            Spinner::Buffer::Pointer ConstantBuffer;
            ConstantBufferType LocalConstantBuffer{};

        public:
            [[nodiscard]] Spinner::ShaderGroup::Pointer GetShaderGroup() const;
            void SetShaderGroup(const Spinner::ShaderGroup::Pointer &shaderGroup);

            Spinner::MeshBuffer::Pointer GetMeshBuffer();
            void SetMeshBuffer(Spinner::MeshBuffer::Pointer newMeshBuffer);

            [[nodiscard]] Spinner::Material::Pointer GetMaterial();
            void SetMaterial(const Spinner::Material::Pointer &material);

            void Update(const std::shared_ptr<DrawCommand> &drawCommand);

            void UpdateConstantBuffer(const ConstantBufferType &constants);
            ConstantBufferType GetMeshConstants() const;

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
