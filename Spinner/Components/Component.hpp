#ifndef SPINNER_COMPONENT_HPP
#define SPINNER_COMPONENT_HPP

#include <memory>
#include <utility>
#include <type_traits>
#include "../Object.hpp"

namespace Spinner
{
    class SceneObject;

    namespace Components
    {
        class Component : public Object
        {
        protected:
            std::weak_ptr<Spinner::SceneObject> SceneObject;

        public:
            inline std::weak_ptr<Spinner::SceneObject> GetSceneObject()
            {
                return SceneObject;
            }
        };

        using ComponentId = int64_t;

        template<typename T>
        inline ComponentId GetComponentId()
        {
            return -1;
        }
    }
}

#endif //SPINNER_COMPONENT_HPP
