#ifndef SPINNER_VERTEXDESCRIPTION_HPP
#define SPINNER_VERTEXDESCRIPTION_HPP

#include <cstddef>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Spinner
{

    struct VertexDescription
    {
        vk::VertexInputBindingDescription BindingDescription;
        std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions;
        size_t PositionOffset;
    };

} // Spinner

#endif //SPINNER_VERTEXDESCRIPTION_HPP
