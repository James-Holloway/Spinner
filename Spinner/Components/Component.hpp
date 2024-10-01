#ifndef SPINNER_COMPONENT_HPP
#define SPINNER_COMPONENT_HPP

#include <memory>
#include <utility>
#include <type_traits>
#include <concepts>
#include "../Object.hpp"

namespace Spinner
{
    class SceneObject;

    namespace Components
    {
        using ComponentId = int64_t;

        class ComponentError : public std::exception
        {
        public:
            inline explicit ComponentError(const std::string &text) : Text("Spinner ComponentError: " + text)
            {
            };

            [[nodiscard]] const char *what() const noexcept override
            {
                return Text.c_str();
            }

        protected:
            std::string Text;
        };

        class Component : public Object
        {
        public:
            using Pointer = std::unique_ptr<Component>;

            explicit Component(const std::weak_ptr<Spinner::SceneObject> &sceneObject, ComponentId componentId, int64_t componentIndex);

        protected:
            std::weak_ptr<Spinner::SceneObject> SceneObject{};
            const Spinner::Components::ComponentId ComponentId = -1;
            const int64_t ComponentIndex = -1;
            std::string Name;
            bool Active = true;

        public:
            [[nodiscard]] std::weak_ptr<Spinner::SceneObject> GetSceneObjectWeak() const;
            [[nodiscard]] std::shared_ptr<Spinner::SceneObject> GetSceneObject() const;
            [[nodiscard]] bool GetActive() const noexcept;
            void SetActive(bool active) noexcept;
            [[nodiscard]] Components::ComponentId GetComponentId() const noexcept;
            [[nodiscard]] int64_t GetComponentIndex() const noexcept;
            [[nodiscard]] std::string GetComponentName() const noexcept;
            void SetComponentName(const std::string &name) noexcept;

            void BaseRenderDebugUI();
        };

        template<typename T>
        concept IsComponent = std::derived_from<T, Spinner::Components::Component>;

        template<IsComponent T>
        inline constexpr ComponentId GetComponentId()
        {
            return -1;
        }

        template<IsComponent T>
        inline const char *GetComponentName()
        {
            return "Component";
        }

        template<IsComponent T>
        inline bool IsComponentType(Component *component)
        {
            if (component == nullptr)
            {
                return false;
            }

            return component->GetComponentId() == GetComponentId<T>();
        }

        template<IsComponent T>
        inline T *AsComponentType(Component *component)
        {
            if (!IsComponentType<T>(component))
            {
                return nullptr;
            }

            return dynamic_cast<T *>(component);
        }

        template<IsComponent T>
        inline void RenderDebugUI(T *component)
        {
            component->BaseRenderDebugUI();
        }
    }
}

#endif //SPINNER_COMPONENT_HPP
