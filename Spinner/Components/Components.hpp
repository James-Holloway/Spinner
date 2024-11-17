#ifndef SPINNER_COMPONENTS_HPP
#define SPINNER_COMPONENTS_HPP

#include "Component.hpp"
#include "MeshComponent.hpp"
#include "CameraComponent.hpp"
#include "LightComponent.hpp"
#include "CameraControllerComponent.hpp"

#define SPINNER_COMPONENT_TEMPLATE_CASE(type, func) case GetComponentId<type>(): return func<type>();

namespace Spinner::Components
{
    inline const char *GetNameByComponentId(ComponentId id)
    {
        switch (id)
        {
            SPINNER_COMPONENT_TEMPLATE_CASE(MeshComponent, GetComponentName);
            SPINNER_COMPONENT_TEMPLATE_CASE(CameraComponent, GetComponentName);
            SPINNER_COMPONENT_TEMPLATE_CASE(LightComponent, GetComponentName);
            SPINNER_COMPONENT_TEMPLATE_CASE(CameraControllerComponent, GetComponentName);

            default:
                return GetComponentName<Component>();
        }
    }
}

#endif //SPINNER_COMPONENTS_HPP
