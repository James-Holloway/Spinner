#ifndef SPINNER_IMGUIINSTANCE_HPP
#define SPINNER_IMGUIINSTANCE_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "imgui.h"
#include "Graphics.hpp"
#include "DescriptorPool.hpp"
#include "Object.hpp"
#include "Graphics.hpp"

namespace Spinner
{
    class ImGuiInstance : public Object
    {
    public:
        using Pointer = std::shared_ptr<ImGuiInstance>;

        explicit ImGuiInstance(const std::shared_ptr<Spinner::Graphics> &graphics);
        ~ImGuiInstance() override;

        static void StartFrame();
        static void EndFrame(const Spinner::CommandBuffer::Pointer &commandBuffer, const vk::ImageView &swapchainImageView);

    protected:
        Spinner::DescriptorPool::Pointer DescriptorPool;
    };
} // Spinner

#endif //SPINNER_IMGUIINSTANCE_HPP
