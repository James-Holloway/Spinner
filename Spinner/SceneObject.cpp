#include "SceneObject.hpp"

#include <utility>
#include "Utilities.hpp"

namespace Spinner
{
    SceneObject::SceneObject(std::string name, bool isScene) : Name(std::move(name)), IsScene(isScene)
    {

    }

    std::string SceneObject::GetName() const
    {
        return Name;
    }

    void SceneObject::SetName(const std::string &name)
    {
        Name = name;
    }

    SceneObject::Pointer SceneObject::GetParent()
    {
        return Parent.lock();
    }

    bool SceneObject::SetParent(const SceneObject::Pointer &newParent)
    {
        if (IsScene)
        {
            return false;
        }
        auto sharedThis = shared_from_this();

        if (newParent == sharedThis)
        {
            throw std::runtime_error("Cannot set a SceneObject as it's own parent");
        }
        if (GetParent() != nullptr)
        {
            GetParent()->RemoveChild(sharedThis);
        }
        Parent = newParent;

        if (newParent != nullptr)
        {
            newParent->AddChild(sharedThis);
        }

        SetWorldMatrixDirty();
        SetSceneParentDirty();

        return true;
    }

    void SceneObject::AddChild(const SceneObject::Pointer &newChild)
    {
        if (newChild == nullptr)
        {
            return;
        }
        auto sharedThis = shared_from_this();
        if (newChild == sharedThis)
        {
            throw std::runtime_error("Cannot add a SceneObject to it's own children");
        }

        if (!Contains(Children, newChild))
        {
            Children.push_back(newChild);
        }

        if (newChild->GetParent() != sharedThis)
        {
            newChild->SetParent(sharedThis);
        }
    }

    bool SceneObject::RemoveChild(const SceneObject::Pointer &child)
    {
        size_t erased = std::erase(Children, child);
        return erased > 0;
    }

    bool SceneObject::IsActive() const
    {
        return Active;
    }

    void SceneObject::SetWorldMatrixDirty()
    {
        DirtyWorldMatrix = true;

        for (auto &child : Children)
        {
            if (child != nullptr)
            {
                child->SetWorldMatrixDirty();
            }
        }
    }

    void SceneObject::SetSceneParentDirty()
    {
        DirtySceneParent = true;

        for (auto &child : Children)
        {
            if (child != nullptr)
            {
                child->SetSceneParentDirty();
            }
        }
    }

    void SceneObject::ConstructMatrix()
    {
        Matrix = glm::translate({}, Position) * glm::mat4_cast(Rotation) * glm::scale({}, Scale);
        DirtyMatrix = false;
    }

    void SceneObject::ConstructWorldMatrix()
    {
        if (IsScene)
        {
            WorldMatrix = glm::mat4{};
        }
        else if (GetParent() != nullptr)
        {
            WorldMatrix = GetParent()->GetWorldMatrix() * GetLocalMatrix();
        }
        else
        {
            WorldMatrix = GetLocalMatrix();
        }

        InverseWorldMatrix = glm::inverse(WorldMatrix);

        DirtyWorldMatrix = false;
    }

    glm::mat4 SceneObject::GetLocalMatrix()
    {
        return Matrix;
    }

    glm::vec3 SceneObject::GetLocalPosition()
    {
        return Position;
    }

    glm::quat SceneObject::GetLocalRotation()
    {
        return Rotation;
    }

    glm::vec3 SceneObject::GetLocalScale()
    {
        return Scale;
    }

    glm::mat4 SceneObject::GetWorldMatrix()
    {
        if (DirtyWorldMatrix)
        {
            ConstructWorldMatrix();
        }

        return WorldMatrix;
    }

    glm::mat4 SceneObject::GetInverseWorldMatrix()
    {
        if (DirtyWorldMatrix)
        {
            ConstructWorldMatrix();
        }

        return InverseWorldMatrix;
    }

} // Spinner