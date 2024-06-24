#include "Scene.hpp"

#include <utility>

namespace Spinner
{
    Scene::Scene(std::string name) : Name(std::move(name))
    {
        ObjectTree = std::make_shared<Spinner::SceneObject>("Scene root", true);
    }

    void Scene::AddObjectToScene(SceneObject::Pointer object, SceneObject::Pointer parent)
    {
        if (object == nullptr)
        {
            return;
        }
        if (parent == nullptr)
        {
            parent = ObjectTree;
        }

        parent->AddChild(object);
    }

    void Scene::SetDescriptorPool(const Spinner::DescriptorPool::Pointer& descriptorPool)
    {
        DescriptorPool = descriptorPool;
    }

} // Spinner