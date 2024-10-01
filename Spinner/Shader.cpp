#include "Shader.hpp"

#include "Utilities.hpp"
#include "Graphics.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"

namespace Spinner
{
    Shader::Shader(const ShaderCreateInfo &createInfo)
    {
        ShaderStage = createInfo.ShaderStage;
        NextStage = createInfo.NextStage;
        ShaderName = createInfo.ShaderName;
        DescriptorSetLayouts = createInfo.DescriptorSetLayouts;

        if (DescriptorSetLayouts.empty())
        {
            throw std::runtime_error("Cannot create a shader with an invalid DescriptorSetLayout from ShaderCreateInfo");
        }

        // Pipeline layout
        auto pushConstants = GetPushConstantRanges(0); // WARNING: Only takes push constants from the first descriptor set layout
        if (createInfo.SceneDescriptorSetLayout != nullptr)
        {
            SceneDescriptorSetIndex = static_cast<uint32_t>(DescriptorSetLayouts.size());
            DescriptorSetLayouts.push_back(createInfo.SceneDescriptorSetLayout);
            SceneDescriptorSetLayout = createInfo.SceneDescriptorSetLayout;
        }
        if (createInfo.LightingDescriptorSetLayout != nullptr)
        {
            LightingDescriptorSetIndex = static_cast<uint32_t>(DescriptorSetLayouts.size());
            DescriptorSetLayouts.push_back(createInfo.LightingDescriptorSetLayout);
            LightingDescriptorSetLayout = createInfo.LightingDescriptorSetLayout;
        }

        std::vector<vk::DescriptorSetLayout> layouts = GetDescriptorSetLayouts();
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayouts(layouts);
        pipelineLayoutCreateInfo.setPushConstantRanges(pushConstants);
        VkPipelineLayout = Graphics::GetDevice().createPipelineLayout(pipelineLayoutCreateInfo);

        if (createInfo.UpdateDrawComponentCallback != nullptr)
        {
            RegisterCallback(UpdateDrawComponentCallback, createInfo.UpdateDrawComponentCallback);
        }
    }

    Shader::~Shader()
    {
        auto &device = Graphics::GetDevice();

        if (VkShader)
        {
            device.destroyShaderEXT(VkShader, nullptr, VulkanInstance::GetDispatchLoader());
        }

        if (VkPipelineLayout)
        {
            device.destroyPipelineLayout(VkPipelineLayout);
            VkPipelineLayout = nullptr;
        }

        SceneDescriptorSetLayout.reset();
        LightingDescriptorSetLayout.reset();
        DescriptorSetLayouts.clear();
    }

    Shader::Pointer Shader::CreateShader(const ShaderCreateInfo &createInfo)
    {
        auto &device = Graphics::GetDevice();

        std::string shaderPath = GetAssetPath(AssetType::Shader, GetShaderFileName(createInfo.ShaderName, createInfo.ShaderStage));

        if (!FileExists(shaderPath))
        {
            throw std::runtime_error("Shader does not exist at path " + shaderPath);
        }

        auto shaderSPIRV = ReadFile(shaderPath);

        auto shader = std::make_shared<Shader>(createInfo);

        auto descriptorSetLayouts = shader->GetDescriptorSetLayouts();

        vk::ShaderCreateInfoEXT shaderCreateInfo;
        shaderCreateInfo.flags = {};
        shaderCreateInfo.stage = createInfo.ShaderStage;
        shaderCreateInfo.nextStage = createInfo.NextStage;
        shaderCreateInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
        shaderCreateInfo.pName = "main";
        shaderCreateInfo.setCode<char>(shaderSPIRV);
        shaderCreateInfo.setSetLayouts(descriptorSetLayouts);
        auto result = device.createShaderEXT(shaderCreateInfo, nullptr, VulkanInstance::GetDispatchLoader());
        vk::detail::resultCheck(result.result, "Could not create shader object");
        shader->VkShader = result.value;

        return shader;
    }

    std::string Shader::GetShaderName() const
    {
        return ShaderName;
    }

    vk::ShaderStageFlagBits Shader::GetShaderStage() const
    {
        return ShaderStage;
    }

    vk::ShaderStageFlags Shader::GetNextStage() const
    {
        return NextStage;
    }

    vk::ShaderEXT Shader::GetVkShader() const
    {
        return VkShader;
    }

    std::vector<vk::DescriptorSetLayoutBinding> Shader::GetDescriptorSetLayoutBindings(uint32_t index) const
    {
        return DescriptorSetLayouts.at(index)->GetDescriptorSetLayoutBindings();
    }

    std::vector<vk::PushConstantRange> Shader::GetPushConstantRanges(uint32_t index) const
    {
        return DescriptorSetLayouts.at(index)->GetPushConstantRanges();
    }

    vk::DescriptorSetLayout Shader::GetDescriptorSetLayout(uint32_t index) const
    {
        return DescriptorSetLayouts.at(index)->GetDescriptorSetLayout();
    }

    std::vector<vk::DescriptorSetLayout> Shader::GetDescriptorSetLayouts() const
    {
        std::vector<vk::DescriptorSetLayout> layouts;
        layouts.reserve(DescriptorSetLayouts.size());
        for (auto &setLayout : DescriptorSetLayouts)
        {
            layouts.push_back(setLayout->GetDescriptorSetLayout());
        }
        return layouts;
    }

    vk::PipelineLayout Shader::GetPipelineLayout() const
    {
        return VkPipelineLayout;
    }

    std::optional<vk::DescriptorType> Shader::GetDescriptorTypeOfBinding(uint32_t binding, uint32_t set) const
    {
        auto layoutBindings = GetDescriptorSetLayoutBindings(set);

        for (const vk::DescriptorSetLayoutBinding &layoutBinding : layoutBindings)
        {
            if (layoutBinding.binding == binding)
            {
                return {layoutBinding.descriptorType};
            }
        }

        return {};
    }

    vk::DescriptorSetLayout Shader::GetSceneDescriptorSetLayout() const
    {
        if (SceneDescriptorSetLayout == nullptr)
        {
            return nullptr;
        }
        return SceneDescriptorSetLayout->GetDescriptorSetLayout();
    }

    vk::DescriptorSetLayout Shader::GetLightingDescriptorSetLayout() const
    {
        if (LightingDescriptorSetLayout == nullptr)
        {
            return nullptr;
        }
        return LightingDescriptorSetLayout->GetDescriptorSetLayout();
    }

    uint32_t Shader::GetSceneDescriptorSetIndex() const
    {
        return SceneDescriptorSetIndex;
    }

    uint32_t Shader::GetLightingDescriptorSetIndex() const
    {
        return LightingDescriptorSetIndex;
    }

    uint32_t Shader::GetBindingFromIndex(uint32_t set, uint32_t indexOfType, vk::DescriptorType type)
    {
        size_t index = 0;
        auto bindings = GetDescriptorSetLayoutBindings(set);
        for (auto &binding : bindings)
        {
            bool sameStage = static_cast<vk::ShaderStageFlags::MaskType>(binding.stageFlags & ShaderStage) != 0;
            bool sameType = (binding.descriptorType == type);
            if (sameStage && sameType)
            {
                if (index == indexOfType)
                {
                    return binding.binding;
                }
                index++;
            }
        }

        return InvalidBindingIndex;
    }

    uint32_t Shader::GetBindingFromIndex(uint32_t set, uint32_t indexOfType, const std::vector<vk::DescriptorType> &types)
    {
        size_t index = 0;
        auto bindings = GetDescriptorSetLayoutBindings(set);
        for (auto &binding : bindings)
        {
            bool sameStage = static_cast<vk::ShaderStageFlags::MaskType>(binding.stageFlags & ShaderStage) != 0;
            bool sameType = Contains(types, binding.descriptorType);
            if (sameStage && sameType)
            {
                if (index == indexOfType)
                {
                    return binding.binding;
                }
                index++;
            }
        }

        return InvalidBindingIndex;
    }

    void ShaderGroup::BindShaders(const CommandBuffer::Pointer &commandBuffer) const
    {
        // Unbind unused shaders
        if (!HasShaderStage(vk::ShaderStageFlagBits::eGeometry))
        {
            commandBuffer->UnbindShaderStage(vk::ShaderStageFlagBits::eGeometry);
        }

        for (auto &shader : Shaders)
        {
            commandBuffer->TrackObject(shader);
            commandBuffer->BindShader(shader);
        }
    }

    void ShaderGroup::RunUpdateDrawComponentCallbacks(const std::shared_ptr<Spinner::DrawCommand> &drawCommand, Components::Component *drawComponent)
    {
        for (const auto &shader : Shaders)
        {
            shader->UpdateDrawComponentCallback.Run(drawCommand, drawComponent);
        }
    }

    bool ShaderGroup::HasShaderStage(const vk::ShaderStageFlagBits shaderStage) const
    {
        for (auto &shader : Shaders)
        {
            if (shader->ShaderStage == shaderStage)
            {
                return true;
            }
        }
        return false;
    }

    Shader::Pointer ShaderGroup::GetShader(const vk::ShaderStageFlagBits shaderStage) const
    {
        for (auto &shader : Shaders)
        {
            if (shader->ShaderStage == shaderStage)
            {
                return shader;
            }
        }
        return nullptr;
    }

    ShaderGroup::Pointer ShaderGroup::CreateShaderGroup(const std::vector<ShaderCreateInfo> &createInfos)
    {
        auto &device = Graphics::GetDevice();

        std::vector<std::string> shaderPaths;
        shaderPaths.reserve(createInfos.size());

        for (const auto &createInfo : createInfos)
        {
            std::string shaderPath = GetAssetPath(AssetType::Shader, GetShaderFileName(createInfo.ShaderName, createInfo.ShaderStage));

            if (!FileExists(shaderPath))
            {
                throw std::runtime_error("Shader does not exist at path " + shaderPath);
            }

            shaderPaths.push_back(shaderPath);
        }

        std::vector<std::vector<char, AlignedAllocator<char, 16>>> shaderSPIRVs;
        shaderSPIRVs.reserve(shaderPaths.size());
        for (auto &shaderPath : shaderPaths)
        {
            shaderSPIRVs.push_back(ReadFile<16>(shaderPath));
        }

        std::vector<Shader::Pointer> shaders;
        shaders.reserve(createInfos.size());
        for (const auto &createInfo : createInfos)
        {
            auto shader = std::make_shared<Shader>(createInfo);
            shaders.push_back(shader);
        }

        std::vector<vk::ShaderCreateInfoEXT> shaderCreateInfos;
        shaderCreateInfos.reserve(createInfos.size());
        std::vector<std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts(shaders.size(), std::vector<vk::DescriptorSetLayout>{});
        for (int i = 0; i < createInfos.size(); i++)
        {
            descriptorSetLayouts[i] = shaders[i]->GetDescriptorSetLayouts();

            vk::ShaderCreateInfoEXT shaderCreateInfo;

            shaderCreateInfo.flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
            shaderCreateInfo.stage = createInfos[i].ShaderStage;
            shaderCreateInfo.nextStage = createInfos[i].NextStage;
            shaderCreateInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
            shaderCreateInfo.pName = "main";
            shaderCreateInfo.pCode = shaderSPIRVs[i].data();
            shaderCreateInfo.codeSize = shaderSPIRVs[i].size();
            shaderCreateInfo.setSetLayouts(descriptorSetLayouts[i]);

            shaderCreateInfos.push_back(shaderCreateInfo);
        }

        auto vkShaders = device.createShadersEXT(shaderCreateInfos, nullptr, VulkanInstance::GetDispatchLoader());
        vk::detail::resultCheck(vkShaders.result, "Could not create shader objects");

        for (int i = 0; i < shaders.size(); i++)
        {
            shaders[i]->VkShader = vkShaders.value[i];
        }

        auto shaderGroup = std::make_shared<ShaderGroup>();
        shaderGroup->Shaders = shaders;

        return shaderGroup;
    }

    std::string GetShaderFileName(const std::string &shaderName, vk::ShaderStageFlagBits stage)
    {
        switch (stage)
        {
            default:
                throw std::runtime_error("Unhandled shader stage");
            case vk::ShaderStageFlagBits::eVertex:
                return shaderName + ".vert.spv";
            case vk::ShaderStageFlagBits::eFragment:
                return shaderName + ".frag.spv";
        }
    }
} // Spinner
