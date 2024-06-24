#ifndef SPINNER_QUEUEFAMILYINDICES_HPP
#define SPINNER_QUEUEFAMILYINDICES_HPP

#include <optional>
#include <cstdint>

namespace Spinner
{

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;
        std::optional<uint32_t> ComputeFamily;
        std::optional<uint32_t> GraphicsFamilyQueueCount;
        std::optional<uint32_t> PresentFamilyQueueCount;
        std::optional<uint32_t> ComputeFamilyQueueCount;

        [[nodiscard]] inline bool IsComplete() const
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value() && ComputeFamily.has_value();
        }
    };

}

#endif //SPINNER_QUEUEFAMILYINDICES_HPP
