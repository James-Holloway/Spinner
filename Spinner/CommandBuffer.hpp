#ifndef SPINNER_COMMANDBUFFER_HPP
#define SPINNER_COMMANDBUFFER_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>
#include <functional>

#include "Callback.hpp"
#include "Object.hpp"
#include "Shader.hpp"

namespace Spinner
{
    class Graphics;

    class MeshBuffer;

    class Image;

    class CommandBuffer : public Object
    {
        friend class Graphics;

    public:
        using Pointer = std::shared_ptr<CommandBuffer>;

        enum class CommandBufferType
        {
            Graphics
        };

        explicit CommandBuffer(vk::CommandBuffer commandBuffer);
        ~CommandBuffer() override = default;

    public:
        vk::CommandBuffer VkCommandBuffer;
        std::vector<std::shared_ptr<void>> TrackedObjects;
        Callback<> OnCompletionCallback;

        CommandBufferType BufferType = CommandBufferType::Graphics;
        bool Recording = false;

    protected:
        void Completed();

    public:
        void TrackObject(const std::shared_ptr<void> &object);
        void CallOnCompletion(std::function<void()> function);

    public:
        void Begin(const vk::CommandBufferBeginInfo &beginInfo);
        vk::Result Begin(const vk::CommandBufferBeginInfo *beginInfo);
        void End();
        void Reset(vk::CommandBufferResetFlags flags = {});

    public:
        void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, const vk::ArrayProxy<const vk::BufferCopy> &regions) const;
        void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, uint32_t regionCount, vk::BufferCopy *regions) const;

        void CopyBufferToImage(const vk::Buffer &srcBuffer, vk::Image &dstImage, vk::ImageLayout dstImageLayout, const vk::ArrayProxy<const vk::BufferImageCopy> &regions) const;
        void CopyBufferToImage(const vk::Buffer &srcBuffer, vk::Image &dstImage, vk::ImageLayout dstImageLayout, uint32_t regionCount, vk::BufferImageCopy *regions) const;

        void BeginRendering(const vk::RenderingInfo &renderingInfo, vk::Extent2D extent, float minDepth = 0.0f, float maxDepth = 1.0f);
        void EndRendering();

        void BindShader(const Shader::Pointer &shader);
        void BindShaderInstance(const ShaderInstance::Pointer &shaderInstance);
        void UnbindShaderStage(vk::ShaderStageFlagBits stage);
        void SetDrawParameters(vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise, vk::PolygonMode polygonMode = vk::PolygonMode::eFill, bool primitiveRestartEnabled = false);
        void SetDepthParameters(vk::CompareOp compareOp = vk::CompareOp::eLessOrEqual, bool depthWrite = true, bool depthTest = true, bool depthBiasEnable = false, float depthBiasConstant = 1.0f, float depthBiasSlope = 0.0f, float depthBiasClamp = 0.0f);

        void DrawMesh(const std::shared_ptr<MeshBuffer> &meshBuffer);
        void BindDescriptors(vk::PipelineLayout layout, uint32_t firstSet, const vk::ArrayProxy<const vk::DescriptorSet> &sets, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics);

        void BindShaderInstanceDescriptors(const ShaderInstance::Pointer &shaderInstance, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics);

        void InsertImageMemoryBarrier(vk::Image image, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::ImageSubresourceRange subresourceRange);
        void TransitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags2 srcStage = vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlags2 dstStage = vk::PipelineStageFlagBits2::eAllCommands, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
        void TransitionImageLayout(const std::shared_ptr<Image> &image, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags2 srcStage = vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlags2 dstStage = vk::PipelineStageFlagBits2::eAllCommands, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
    };

} // Spinner

#endif //SPINNER_COMMANDBUFFER_HPP
