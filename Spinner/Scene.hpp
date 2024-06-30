#ifndef SPINNER_SCENE_HPP
#define SPINNER_SCENE_HPP

#include "Object.hpp"
#include "SceneObject.hpp"
#include "DescriptorPool.hpp"

namespace Spinner
{
    class Graphics;

    class Lighting;

    class Scene : public Object, public std::enable_shared_from_this<Scene>
    {
        friend class Graphics;

    public:
        using Pointer = std::shared_ptr<Scene>;

        explicit Scene(std::string name);
        ~Scene() override = default;

    public:
        bool AddObjectToScene(const SceneObject::Pointer &object, SceneObject::Pointer parent = nullptr);
        void Update(uint32_t currentFrame);
        void Draw(uint32_t currentFrame, CommandBuffer::Pointer &commandBuffer);
        [[nodiscard]] bool IsActive() const noexcept;
        void SetActive(bool active);
        Buffer::Pointer GetSceneBuffer();
        [[nodiscard]] SceneConstants GetSceneConstants() const noexcept;
        void UpdateSceneConstants(const SceneConstants &sceneConstants) noexcept;
        [[nodiscard]] std::shared_ptr<Spinner::Lighting> GetLighting() const noexcept;

        [[nodiscard]] SceneObject::Pointer GetObjectTree();

    protected:
        std::string Name;
        SceneObject::Pointer ObjectTree;
        Buffer::Pointer SceneBuffer;
        SceneConstants LocalSceneBuffer{};
        std::shared_ptr<Spinner::Lighting> Lighting;

        bool Active = true;
        bool HasSetObjectTreeScene = false;

    protected:
        static std::weak_ptr<Spinner::Lighting> GlobalLighting;

    public:
        static SceneObject::Pointer LoadModel(const std::string &modelFilename);
        [[nodiscard]] static std::shared_ptr<Spinner::Lighting> GetGlobalLighting();
    };

} // Spinner

#endif //SPINNER_SCENE_HPP
