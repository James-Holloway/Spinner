#ifndef SPINNER_SCENEOBJECT_HPP
#define SPINNER_SCENEOBJECT_HPP

#include <memory>
#include <string>

#include "Object.hpp"
#include "GLM.hpp"
#include "Components/MeshComponent.hpp"

namespace Spinner
{
    class Scene;

    class SceneObject;

    // I don't like declaring this here but there are circular dependencies
    namespace Components
    {
        template<IsComponent T>
        class ComponentPtr
        {
            friend class SceneObject;

        public:
            ComponentPtr() : ComponentIndex(0)
            {
            }

            inline explicit ComponentPtr(const std::weak_ptr<SceneObject> &sceneObject, int64_t componentIndex)
            {
                SceneObject = sceneObject;
                ComponentIndex = componentIndex;
            }

            [[nodiscard]] inline std::shared_ptr<SceneObject> GetOwnerSceneObject() const
            {
                return SceneObject.lock();
            }

            [[nodiscard]] inline bool IsValid() const noexcept
            {
                return !SceneObject.expired();
            }

            // Declared further down as it requires access to SceneObject's templated functions
            T *operator->() const;

        protected:
            /// Used internally by SceneObject
            [[nodiscard]] inline int64_t GetComponentIndex() const noexcept
            {
                return ComponentIndex;
            }

        protected:
            std::weak_ptr<SceneObject> SceneObject;
            int64_t ComponentIndex;
        };
    }

    class SceneObject : public Object, public std::enable_shared_from_this<SceneObject>
    {
        friend class Spinner::Scene;

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

        [[nodiscard]] Pointer GetParent() const;
        bool SetParent(const Pointer &newParent);

        void AddChild(const Pointer &newChild);
        bool RemoveChild(const Pointer &child);

        bool IsActive() const;
        void SetActive(bool active);

        void Traverse(const std::function<bool(const SceneObject::Pointer &)> &traverseFunction, const std::function<void(const SceneObject::Pointer &)> &traverseUpFunction = nullptr, int maxDepth = -1, int currentDepth = 0);
        void TraverseActive(const std::function<bool(const SceneObject::Pointer &)> &traverseFunction, const std::function<void(const SceneObject::Pointer &)> &traverseUpFunction = nullptr, int maxDepth = -1, int currentDepth = 0);

        glm::mat4 GetLocalMatrix();
        glm::vec3 GetLocalPosition();
        glm::quat GetLocalRotation();
        glm::vec3 GetLocalEulerRotation();
        glm::vec3 GetLocalScale();

        void SetLocalMatrix(const glm::mat4 &matrix);
        void SetLocalPosition(glm::vec3 position);
        void SetLocalRotation(glm::quat rotation);
        void SetLocalEulerRotation(glm::vec3 eulerRotation);
        void SetLocalScale(glm::vec3 scale);

        glm::mat4 GetWorldMatrix();
        glm::mat4 GetInverseWorldMatrix();
        glm::vec3 GetWorldPosition();
        glm::quat GetWorldRotation();
        glm::vec3 GetWorldScale();

        void SetWorldMatrix(const glm::mat4 &matrix);
        void SetWorldPosition(glm::vec3 position);
        void SetWorldRotation(glm::quat rotation);
        void SetWorldScale(glm::vec3 scale);

        std::shared_ptr<Scene> GetSceneParent();

        [[nodiscard]] std::vector<Pointer> GetChildren() const;
        size_t GetChildrenCount() const;
        Pointer GetChildByIndex(size_t index) const;
        Pointer GetFirstChildWithName(const std::string &name) const;

    protected:
        void SetWorldMatrixDirty();
        void SetSceneParentDirty();
        void ConstructMatrix();
        void ConstructWorldMatrix();

    public:
        // Components

        bool HasComponent(Components::ComponentId componentId) const;

        template<Components::IsComponent T>
        inline bool HasComponent()
        {
            return HasComponent(Components::GetComponentId<T>());
        }

        size_t ComponentCount(Components::ComponentId componentId) const;

        template<Components::IsComponent T>
        inline size_t ComponentCount()
        {
            return ComponentCount(Components::GetComponentId<T>());
        }

        inline size_t TotalComponentCount()
        {
            return Components.size();
        }

        template<Components::IsComponent T>
        inline Components::ComponentPtr<T> AddComponent()
        {
            Components.push_back(std::make_unique<T>(weak_from_this(), ComponentIndexCounter));
            auto componentPtr = Components::ComponentPtr<T>(weak_from_this(), ComponentIndexCounter);

            ComponentIndexCounter++;

            return componentPtr;
        }

        template<Components::IsComponent T>
        T *GetFirstComponentRawPointer()
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id)
                {
                    return reinterpret_cast<T *>(component.get());
                }
            }
            return nullptr;
        }

        template<Components::IsComponent T>
        T *GetComponentRawPointer(int64_t componentIndex)
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id && component->GetComponentIndex() == componentIndex)
                {
                    return reinterpret_cast<T *>(component.get());
                }
            }
            return nullptr;
        }

        template<Components::IsComponent T>
        std::vector<T *> GetComponentRawPointers()
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            std::vector<T *> rawPointers;
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id)
                {
                    rawPointers.push_back(reinterpret_cast<T *>(component.get()));
                }
            }

            return rawPointers;
        }

        template<Components::IsComponent T>
        void GetComponentRawPointers(std::vector<T *> &rawPointers)
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id)
                {
                    rawPointers.push_back(reinterpret_cast<T *>(component.get()));
                }
            }
        }

        template<Components::IsComponent T>
        std::optional<Components::ComponentPtr<T>> GetFirstComponent()
        {
            auto ptr = GetFirstComponentRawPointer<T>();
            if (ptr == nullptr)
            {
                return {};
            }
            return {Components::ComponentPtr<T>(shared_from_this(), ptr->GetComponentIndex())};
        }

        template<Components::IsComponent T>
        std::vector<Components::ComponentPtr<T>> GetComponents()
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            std::vector<Components::ComponentPtr<T>> components;
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id)
                {
                    components.emplace_back(shared_from_this(), component->GetComponentIndex());
                }
            }

            return components;
        }

        template<Components::IsComponent T>
        void GetComponents(std::vector<Components::ComponentPtr<T>> &components)
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            for (auto &component : Components)
            {
                if (component->GetComponentId() == id)
                {
                    components.emplace_back(shared_from_this(), component->GetComponentIndex());
                }
            }
        }

        template<Components::IsComponent T>
        inline void GetComponentsInChildren(std::vector<Components::ComponentPtr<T>> &components, bool includeInactive = false)
        {
            for (auto &child : Children)
            {
                if (includeInactive || child->IsActive())
                {
                    child->GetComponents(components);
                }
            }
        }

        template<Components::IsComponent T>
        inline std::vector<Components::ComponentPtr<T>> GetComponentsInChildren(bool includeInactive = false)
        {
            std::vector<Components::ComponentPtr<T>> components;
            GetComponentsInChildren<T>(components, includeInactive);
            return components;
        }

        template<Components::IsComponent T>
        inline void GetComponentRawPointersInChildren(std::vector<T *> &components, bool includeInactive = false)
        {
            for (auto &child : Children)
            {
                if (includeInactive || child->IsActive())
                {
                    child->GetComponentRawPointers<T>(components);
                }
            }
        }

        template<Components::IsComponent T>
        inline std::vector<Components::ComponentPtr<T>> GetComponentRawPointersInChildren(bool includeInactive = false)
        {
            std::vector<T *> components;
            GetComponentRawPointersInChildren<T>(components, includeInactive);
            return components;
        }

        template<Components::IsComponent T>
        inline void RemoveComponent(Components::ComponentPtr<T> componentPtr)
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            int64_t componentIndex = componentPtr->GetComponentIndex();
            std::remove_if(Components.begin(), Components.end(), [&id, &componentIndex](const Components::Component::Pointer &component)
            {
                return component->GetComponentId() == id && component->GetComponentIndex() == componentIndex;
            });
        }

        template<Components::IsComponent T>
        inline void RemoveComponent(T *componentRawPtr)
        {
            Components::ComponentId id = Components::GetComponentId<T>();
            int64_t componentIndex = componentRawPtr->GetComponentIndex();
            std::remove_if(Components.begin(), Components.end(), [&id, &componentIndex](const Components::Component::Pointer &component)
            {
                return component->GetComponentId() == id && component->GetComponentIndex() == componentIndex;
            });
        }

        void RenderDebugUI();

    public:
        Callback<const SceneObject::Pointer &, const std::weak_ptr<Scene> &> ParentChanged;

    protected:
        std::string Name;

        WeakPointer Parent{};
        std::weak_ptr<Spinner::Scene> SceneParent{};
        std::vector<Pointer> Children{};

        std::vector<Components::Component::Pointer> Components;
        std::atomic<int64_t> ComponentIndexCounter = 0;

        glm::vec3 Position{0, 0, 0};
        glm::quat Rotation{1, 0, 0, 0};
        glm::vec3 EulerRotation{0, 0, 0}; // Operates in degrees
        glm::vec3 Scale{1, 1, 1};
        glm::mat4 Matrix{1.0f};
        glm::mat4 WorldMatrix{1.0f};
        glm::mat4 InverseWorldMatrix{1.0f};

        bool Active = true;
        const bool IsScene = false;
        bool DirtySceneParent = true;
        bool DirtyMatrix = true;
        bool DirtyWorldMatrix = true;

    public:
        static Pointer Create(const std::string &name);
    };

    // Implementation that was mentioned further up
    template<Components::IsComponent T>
    T *Components::ComponentPtr<T>::operator->() const
    {
        if (SceneObject.expired())
        {
            throw ComponentError("Owner SceneObject of Component no longer exists");
        }

        return SceneObject.lock()->GetComponentRawPointer<T>(ComponentIndex);
    }
} // Spinner

#endif //SPINNER_SCENEOBJECT_HPP
