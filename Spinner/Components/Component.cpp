#include "Component.hpp"

#include "../SceneObject.hpp"

namespace Spinner
{
    Components::Component::Component(const std::weak_ptr<Spinner::SceneObject> &sceneObject, Spinner::Components::ComponentId componentId, int64_t componentIndex) : SceneObject(sceneObject), ComponentId(componentId), ComponentIndex(componentIndex)
    {

    }

    std::weak_ptr<Spinner::SceneObject> Components::Component::GetSceneObjectWeak() const
    {
        return SceneObject;
    }

    std::shared_ptr<Spinner::SceneObject> Components::Component::GetSceneObject() const
    {
        return SceneObject.lock();
    }

    bool Components::Component::GetActive() const noexcept
    {
        return Active;
    }

    void Components::Component::SetActive(bool active) noexcept
    {
        Active = active;
    }

    Components::ComponentId Components::Component::GetComponentId() const noexcept
    {
        return ComponentId;
    }

    int64_t Components::Component::GetComponentIndex() const noexcept
    {
        return ComponentIndex;
    }

    std::string Components::Component::GetComponentName() const noexcept
    {
        return Name;
    }

    void Components::Component::SetComponentName(const std::string &name) noexcept
    {
        Name = name;
    }
}