#ifndef SPINNER_SCENEOBJECT_HPP
#define SPINNER_SCENEOBJECT_HPP

#include <memory>
#include <string>

#include "Object.hpp"
#include "GLM.hpp"

namespace Spinner
{
    class Scene;

    class SceneObject : public Object, public std::enable_shared_from_this<SceneObject>
    {
        friend class Scene;

    public:
        using Pointer = std::shared_ptr<SceneObject>;
        using WeakPointer = std::weak_ptr<SceneObject>;

    public:
        constexpr inline static std::string DefaultName = "Unnamed Object";
        explicit SceneObject(std::string name = DefaultName, bool isScene = false);
        ~SceneObject() override = default;

    public:
        [[nodiscard]] std::string GetName() const;
        void SetName(const std::string &name);

        [[nodiscard]] Pointer GetParent();
        bool SetParent(const Pointer &newParent);

        void AddChild(const Pointer &newChild);
        bool RemoveChild(const Pointer &child);

        bool IsActive() const;

        glm::mat4 GetLocalMatrix();
        glm::vec3 GetLocalPosition();
        glm::quat GetLocalRotation();
        glm::vec3 GetLocalScale();

        glm::mat4 GetWorldMatrix();
        glm::mat4 GetInverseWorldMatrix();

    protected:
        void SetWorldMatrixDirty();
        void SetSceneParentDirty();
        void ConstructMatrix();
        void ConstructWorldMatrix();

    protected:
        std::string Name;

        WeakPointer Parent{};
        std::weak_ptr<Scene> SceneParent{};
        std::vector<Pointer> Children{};

        glm::vec3 Position{0, 0, 0};
        glm::quat Rotation{0, 0, 0, 1};
        glm::vec3 Scale{1, 1, 1};
        glm::mat4 Matrix{};
        glm::mat4 WorldMatrix{};
        glm::mat4 InverseWorldMatrix{};

        bool Active = true;
        const bool IsScene = false;
        bool DirtySceneParent = true;
        bool DirtyMatrix = true;
        bool DirtyWorldMatrix = true;
    };

} // Spinner

#endif //SPINNER_SCENEOBJECT_HPP
