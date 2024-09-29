#include "SceneObject.hpp"

#include <utility>
#include "Utilities.hpp"
#include "Components/Components.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

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

    SceneObject::Pointer SceneObject::GetParent() const
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

        ParentChanged.Run(sharedThis, GetSceneParent());

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

    void SceneObject::SetActive(bool active)
    {
        Active = active;
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
        Matrix = glm::translate(glm::mat4{1.0f}, Position) * glm::mat4_cast(Rotation) * glm::eulerAngleYXZ(glm::radians(EulerRotation.y), glm::radians(EulerRotation.x), glm::radians(EulerRotation.z)) * glm::scale(glm::mat4{1.0f}, Scale);
        DirtyMatrix = false;
    }

    void SceneObject::ConstructWorldMatrix()
    {
        if (IsScene)
        {
            WorldMatrix = glm::mat4{1.0f};
        }
        else if (GetParent() != nullptr)
        {
            WorldMatrix = GetLocalMatrix() * GetParent()->GetWorldMatrix();
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
        if (DirtyMatrix)
        {
            ConstructMatrix();
        }

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

    glm::vec3 SceneObject::GetLocalEulerRotation()
    {
        return EulerRotation;
    }

    glm::vec3 SceneObject::GetLocalScale()
    {
        return Scale;
    }

    void SceneObject::SetLocalPosition(glm::vec3 position)
    {
        if (IsScene)
            return;
        Position = position;
        DirtyMatrix = true;
        SetWorldMatrixDirty();
    }

    void SceneObject::SetLocalRotation(glm::quat rotation)
    {
        if (IsScene)
            return;
        Rotation = rotation;
        DirtyMatrix = true;
        SetWorldMatrixDirty();
    }

    void SceneObject::SetLocalEulerRotation(glm::vec3 eulerRotation)
    {
        if (IsScene)
        {
            return;
        }

        EulerRotation = eulerRotation;
        DirtyMatrix = true;
        SetWorldMatrixDirty();
    }

    void SceneObject::SetLocalScale(glm::vec3 scale)
    {
        if (IsScene)
            return;
        Scale = scale;
        DirtyMatrix = true;
        SetWorldMatrixDirty();
    }

    void SceneObject::SetLocalMatrix(const glm::mat4 &matrix)
    {
        if (IsScene)
            return;

        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(matrix, scale, rotation, position, skew, perspective);
        Position = position;
        Rotation = rotation;
        Scale = scale;
        EulerRotation = {0, 0, 0};

        DirtyMatrix = true;
        SetWorldMatrixDirty();
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

    glm::vec3 SceneObject::GetWorldPosition()
    {
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(GetWorldMatrix(), scale, rotation, position, skew, perspective);
        return position;
    }

    glm::quat SceneObject::GetWorldRotation()
    {
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(GetWorldMatrix(), scale, rotation, position, skew, perspective);
        return rotation;
    }

    glm::vec3 SceneObject::GetWorldScale()
    {
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(GetWorldMatrix(), scale, rotation, position, skew, perspective);
        return scale;
    }

    void SceneObject::SetWorldMatrix(const glm::mat4 &matrix)
    {
        auto parent = GetParent();
        if (parent == nullptr)
        {
            SetLocalMatrix(matrix);
            return;
        }
        SetLocalMatrix(parent->GetInverseWorldMatrix() * matrix);
    }

    void SceneObject::SetWorldPosition(const glm::vec3 position)
    {
        auto parent = GetParent();
        if (parent == nullptr)
        {
            SetLocalPosition(position);
            return;
        }

        const glm::vec3 newPosition = parent->GetInverseWorldMatrix() * glm::vec4{position, 1.0f}; // Implicit cast of vec4 to vec3
        SetLocalPosition(newPosition);
    }

    void SceneObject::SetWorldRotation(const glm::quat rotation)
    {
        auto parent = GetParent();
        if (parent == nullptr)
        {
            SetLocalRotation(rotation);
            return;
        }

        const glm::quat newRotation = rotation * glm::inverse(parent->GetWorldRotation());
        SetLocalRotation(newRotation);
    }

    void SceneObject::SetWorldScale(const glm::vec3 scale)
    {
        auto parent = GetParent();
        if (parent == nullptr)
        {
            SetLocalScale(scale);
            return;
        }

        SetLocalScale(scale / parent->GetWorldScale());
    }

    bool SceneObject::HasComponent(Components::ComponentId componentId) const
    {
        for (const auto &component : Components)
        {
            if (component->GetComponentId() == componentId)
            {
                return true;
            }
        }
        return false;
    }

    size_t SceneObject::ComponentCount(Components::ComponentId componentId) const
    {
        size_t count = 0;
        for (const auto &component : Components)
        {
            if (component->GetComponentId() == componentId)
            {
                count++;
            }
        }

        return count;
    }

    void SceneObject::Traverse(const std::function<bool(const SceneObject::Pointer &)> &traverseFunction, const std::function<void(const SceneObject::Pointer &)> &traverseUpFunction, int maxDepth, int currentDepth)
    {
        if (traverseFunction == nullptr)
            return;

        if (!IsScene)
        {
            bool result = traverseFunction(shared_from_this());
            if (!result)
                return;
        }

        if (currentDepth >= maxDepth && maxDepth > 0)
            return;

        for (auto &child : Children)
        {
            child->Traverse(traverseFunction, traverseUpFunction, maxDepth, currentDepth + 1);
        }

        if (!IsScene && traverseUpFunction)
        {
            traverseUpFunction(shared_from_this());
        }
    }

    void SceneObject::TraverseActive(const std::function<bool(const SceneObject::Pointer &)> &traverseFunction, const std::function<void(const SceneObject::Pointer &)> &traverseUpFunction, int maxDepth, int currentDepth)
    {
        if (traverseFunction == nullptr)
            return;

        if (!IsScene && IsActive())
        {
            bool result = traverseFunction(shared_from_this());
            if (!result)
                return;
        }

        if (currentDepth >= maxDepth && maxDepth > 0)
            return;

        for (auto &child : Children)
        {
            child->TraverseActive(traverseFunction, traverseUpFunction, maxDepth, currentDepth + 1);
        }

        if (!IsScene && traverseUpFunction)
        {
            traverseUpFunction(shared_from_this());
        }
    }

    std::shared_ptr<Scene> SceneObject::GetSceneParent()
    {
        if (IsScene)
        {
            return SceneParent.lock();
        }
        if (!DirtySceneParent)
        {
            return SceneParent.lock();
        }

        auto parent = GetParent();
        if (parent == nullptr)
        {
            return nullptr;
        }

        auto scene = parent->GetSceneParent();
        DirtySceneParent = false;
        SceneParent = scene;
        return scene;
    }

    SceneObject::Pointer SceneObject::Create(const std::string& name)
    {
        return std::make_shared<Spinner::SceneObject>(name);
    }

    std::vector<SceneObject::Pointer> SceneObject::GetChildren() const
    {
        return Children;
    }

    SceneObject::Pointer SceneObject::GetFirstChildWithName(const std::string &name) const
    {
        for (auto &child : Children)
        {
            if (child->GetName() == name)
            {
                return child;
            }
        }
        return nullptr;
    }

    size_t SceneObject::GetChildrenCount() const
    {
        return Children.size();
    }

    SceneObject::Pointer SceneObject::GetChildByIndex(size_t index) const
    {
        return Children.at(index);
    }

    // NOTE: Add further
    void SceneObject::RenderDebugUI()
    {
        ImGui::SeparatorText("SceneObject");

        bool active = IsActive();
        if (ImGui::Checkbox("Active", &active))
        {
            SetActive(active);
        }

        std::string name = GetName();
        name.reserve(name.size() + 1);
        if (ImGui::InputText("Name", &name))
        {
            SetName(name);
        }

        glm::vec3 localPosition = GetLocalPosition();
        if (ImGui::DragFloat3("Local Position", &localPosition.x, 0.05f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
        {
            SetLocalPosition(localPosition);
        }

        glm::quat localRotation = GetLocalRotation();
        if (ImGui::DragFloat4("Local Rotation", &localRotation.x, 0.01f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
        {
            SetLocalRotation(localRotation);
        }

        glm::vec3 localEulerRotation = GetLocalEulerRotation();
        if (ImGui::DragFloat3("Local Euler Rotation", &localEulerRotation.x, 0.1f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
        {
            SetLocalEulerRotation(localEulerRotation);
        }

        glm::vec3 localScale = GetLocalScale();
        if (ImGui::DragFloat3("Local Scale", &localScale.x, 0.01f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
        {
            SetLocalScale(localScale);
        }

        ImGui::Text("Child count: %lu", Children.size());

        ImGui::Indent(8);
        int componentsRendered = 0;

#define COMPONENT_RENDER_DEBUG_UI(type) \
        auto components##type = GetComponentRawPointers<Components::type>(); \
        for (auto &component : components##type) \
        { \
            ImGui::PushID(componentsRendered++); \
            Components::RenderDebugUI(component); \
            ImGui::PopID(); \
        }

        COMPONENT_RENDER_DEBUG_UI(MeshComponent);
        COMPONENT_RENDER_DEBUG_UI(CameraComponent);
        COMPONENT_RENDER_DEBUG_UI(LightComponent);

#undef COMPONENT_RENDER_DEBUG_UI

        ImGui::Unindent(8);
    }
} // Spinner
