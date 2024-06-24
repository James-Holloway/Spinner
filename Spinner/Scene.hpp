#ifndef SPINNER_SCENE_HPP
#define SPINNER_SCENE_HPP

#include "Object.hpp"
#include "SceneObject.hpp"
#include "DescriptorPool.hpp"

namespace Spinner
{
    class Graphics;

    class Scene : public Object
    {
        friend class Graphics;

    public:
        explicit Scene(std::string name);
        ~Scene() override = default;

    public:
        void AddObjectToScene(SceneObject::Pointer object, SceneObject::Pointer parent = nullptr);

        void SetDescriptorPool(const Spinner::DescriptorPool::Pointer &descriptorPool);


    protected:
        std::string Name;

        DescriptorPool::Pointer DescriptorPool;
        SceneObject::Pointer ObjectTree;

        bool IsActive = false;
    };

} // Spinner

#endif //SPINNER_SCENE_HPP
