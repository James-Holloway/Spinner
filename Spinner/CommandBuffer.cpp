#include "CommandBuffer.hpp"

#include <utility>
#include "VulkanInstance.hpp"
#include "VulkanUtilities.hpp"
#include "MeshBuffer.hpp"

namespace Spinner
{
    CommandBuffer::CommandBuffer(vk::CommandBuffer commandBuffer)
    {
        VkCommandBuffer = commandBuffer;
    }

    void CommandBuffer::TrackObject(const std::shared_ptr<void> &object)
    {
        TrackedObjects.push_back(object);
    }

    void CommandBuffer::CallOnCompletion(std::function<void()> function)
    {
        OnCompletionCallback.Register(std::move(function), {});
    }

    void CommandBuffer::Begin(const vk::CommandBufferBeginInfo &beginInfo)
    {
        if (Recording)
        {
            throw std::runtime_error("Cannot Begin a CommandBuffer which is already Recording");
        }

        VkCommandBuffer.begin(beginInfo);
        Recording = true;
    }

    void CommandBuffer::End()
    {
        if (!Recording)
        {
            throw std::runtime_error("Cannot End a CommandBuffer which is not Recording");
        }

        VkCommandBuffer.end();
        Recording = false;
    }

    void CommandBuffer::Reset(vk::CommandBufferResetFlags flags)
    {
        VkCommandBuffer.reset(flags);

        Recording = false;
    }

    vk::Result CommandBuffer::Begin(const vk::CommandBufferBeginInfo *beginInfo)
    {
        if (Recording)
        {
            throw std::runtime_error("Cannot Begin a CommandBuffer which is already Recording");
        }

        auto result = VkCommandBuffer.begin(beginInfo);
        Recording = true;

        return result;
    }

    void CommandBuffer::Completed()
    {
        OnCompletionCallback.Run();
        OnCompletionCallback.ClearCallbacks();
        TrackedObjects.clear();
    }

    void CommandBuffer::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, const vk::ArrayProxy<const vk::BufferCopy> &regions) const
    {
        VkCommandBuffer.copyBuffer(srcBuffer, dstBuffer, regions);
    }

    void CommandBuffer::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, uint32_t regionCount, vk::BufferCopy *regions) const
    {
        VkCommandBuffer.copyBuffer(srcBuffer, dstBuffer, regionCount, regions);
    }

    void CommandBuffer::BeginRendering(const vk::RenderingInfo &renderingInfo, vk::Extent2D extent, float minDepth, float maxDepth)
    {
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#shaders-objects-state

        VkCommandBuffer.beginRendering(renderingInfo);

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), minDepth, maxDepth);
        vk::Rect2D scissor({0, 0}, extent);
        vk::SampleMask sampleMask = 0xFF;
        std::vector<VkBool32> colorBlendEnables{renderingInfo.colorAttachmentCount, true};
        std::vector<vk::ColorComponentFlags> colorBlendComponentFlags = {renderingInfo.colorAttachmentCount, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
        float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        // Alpha blending equation
        vk::ColorBlendEquationEXT colorBlendEquation;
        colorBlendEquation.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendEquation.dstColorBlendFactor = vk::BlendFactor::eOneMinusConstantAlpha;
        colorBlendEquation.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendEquation.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendEquation.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendEquation.alphaBlendOp = vk::BlendOp::eAdd;

        std::vector<vk::ColorBlendEquationEXT> colorBlendEquations = {renderingInfo.colorAttachmentCount, colorBlendEquation};

        // Set initial state
        VkCommandBuffer.setViewportWithCount(viewport);
        VkCommandBuffer.setScissorWithCount(scissor);
        VkCommandBuffer.setRasterizerDiscardEnable(false);
        VkCommandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setAlphaToCoverageEnableEXT(false, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setStencilTestEnable(false); // need to set stencilOp, stencilCompareMask, stencilWriteMask and stencilReference if true
        VkCommandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, sampleMask, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setColorBlendEnableEXT(0, colorBlendEnables, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setColorWriteMaskEXT(0, colorBlendComponentFlags, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setColorBlendEquationEXT(0, colorBlendEquations, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setBlendConstants(blendConstants);
        // Use default parameters to set more initial state
        SetDrawParameters();
        SetDepthParameters();
    }

    void CommandBuffer::EndRendering()
    {
        VkCommandBuffer.endRendering();
    }

    void CommandBuffer::BindShader(const Shader::Pointer &shader)
    {
        VkCommandBuffer.bindShadersEXT(shader->ShaderStage, shader->VkShader, VulkanInstance::GetDispatchLoader());
    }

    void CommandBuffer::BindShaderInstance(const ShaderInstance::Pointer &shaderInstance)
    {
        BindShader(shaderInstance->Shader);
        BindShaderInstanceDescriptors(shaderInstance);
    }

    void CommandBuffer::UnbindShaderStage(vk::ShaderStageFlagBits stage)
    {
        VkCommandBuffer.bindShadersEXT(stage, {nullptr}, VulkanInstance::GetDispatchLoader());
    }

    void CommandBuffer::SetDrawParameters(vk::CullModeFlags cullMode, vk::PrimitiveTopology topology, vk::FrontFace frontFace, vk::PolygonMode polygonMode, bool primitiveRestartEnabled)
    {
        VkCommandBuffer.setCullMode(cullMode);
        VkCommandBuffer.setPrimitiveTopology(topology);
        VkCommandBuffer.setFrontFace(frontFace);
        VkCommandBuffer.setPolygonModeEXT(polygonMode, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.setPrimitiveRestartEnable(primitiveRestartEnabled);

        if (polygonMode == vk::PolygonMode::eLine)
        {
            VkCommandBuffer.setLineWidth(1.0f);
        }
    }

    void CommandBuffer::SetDepthParameters(vk::CompareOp compareOp, bool depthWrite, bool depthTest, bool depthBiasEnable, float depthBiasConstant, float depthBiasSlope, float depthBiasClamp)
    {
        VkCommandBuffer.setDepthCompareOp(compareOp);
        VkCommandBuffer.setDepthWriteEnable(depthWrite);
        VkCommandBuffer.setDepthTestEnable(depthTest);
        VkCommandBuffer.setDepthBiasEnable(depthBiasEnable);
        VkCommandBuffer.setDepthBias(depthBiasConstant, depthBiasClamp, depthBiasSlope);
    }

    void CommandBuffer::DrawMesh(const std::shared_ptr<MeshBuffer> &meshBuffer)
    {
        // Binding
        VkCommandBuffer.setVertexInputEXT(meshBuffer->VertexBindingDescription, meshBuffer->VertexAttributeDescriptions, VulkanInstance::GetDispatchLoader());
        VkCommandBuffer.bindVertexBuffers(0, meshBuffer->VkBuffer, {meshBuffer->VertexDataOffset});
        VkCommandBuffer.bindIndexBuffer(meshBuffer->VkBuffer, meshBuffer->IndexDataOffset, GetVkIndexType<MeshBuffer::IndexType>());
        // Drawing
        VkCommandBuffer.drawIndexed(meshBuffer->IndexCount, 1, 0, 0, 0);
    }

    void CommandBuffer::BindDescriptors(vk::PipelineLayout layout, uint32_t firstSet, const vk::ArrayProxy<const vk::DescriptorSet> &sets, vk::PipelineBindPoint bindPoint)
    {
        VkCommandBuffer.bindDescriptorSets(bindPoint, layout, firstSet, sets, nullptr);
    }

    void CommandBuffer::BindShaderInstanceDescriptors(const ShaderInstance::Pointer &shaderInstance, vk::PipelineBindPoint bindPoint)
    {
        BindDescriptors(shaderInstance->VkPipelineLayout, 0, shaderInstance->VkDescriptorSets, bindPoint);
    }

    void CommandBuffer::InsertImageMemoryBarrier(vk::Image image, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::ImageSubresourceRange subresourceRange)
    {
        vk::ImageMemoryBarrier2 imageMemoryBarrier;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.srcStageMask = srcStageMask;
        imageMemoryBarrier.dstStageMask = dstStageMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.setImageMemoryBarriers(imageMemoryBarrier);

        VkCommandBuffer.pipelineBarrier2(dependencyInfo);
    }


} // Spinner