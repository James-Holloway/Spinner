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
        DescriptorSetLayout = createInfo.DescriptorSetLayout;

        if (DescriptorSetLayout == nullptr)
        {
            throw std::runtime_error("Cannot create a shader with an invalid DescriptorSetLayout from ShaderCreateInfo");
        }
    }

    Shader::~Shader()
    {
        auto &device = Graphics::GetDevice();

        if (VkShader)
        {
            device.destroyShaderEXT(VkShader, nullptr, VulkanInstance::GetDispatchLoader());
        }

        DescriptorSetLayout.reset();
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

        vk::ShaderCreateInfoEXT shaderCreateInfo;
        shaderCreateInfo.flags = {};
        shaderCreateInfo.stage = createInfo.ShaderStage;
        shaderCreateInfo.nextStage = createInfo.NextStage;
        shaderCreateInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
        shaderCreateInfo.pName = "main";
        shaderCreateInfo.setCode<char>(shaderSPIRV);
        shaderCreateInfo.setSetLayouts(shader->GetDescriptorSetLayout());
        shader->VkShader = device.createShaderEXT(shaderCreateInfo, nullptr, VulkanInstance::GetDispatchLoader());

        return shader;
    }

    std::vector<Shader::Pointer> Shader::CreateLinkedShaders(const std::vector<ShaderCreateInfo> &createInfos)
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
        for (int i = 0; i < createInfos.size(); i++)
        {
            vk::ShaderCreateInfoEXT shaderCreateInfo;

            shaderCreateInfo.flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
            shaderCreateInfo.stage = createInfos[i].ShaderStage;
            shaderCreateInfo.nextStage = createInfos[i].NextStage;
            shaderCreateInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
            shaderCreateInfo.pName = "main";
            shaderCreateInfo.pCode = shaderSPIRVs[i].data();
            shaderCreateInfo.codeSize = shaderSPIRVs[i].size();
            shaderCreateInfo.setSetLayouts(shaders[i]->GetDescriptorSetLayout());

            shaderCreateInfos.push_back(shaderCreateInfo);
        }

        auto vkShaders = device.createShadersEXT(shaderCreateInfos, nullptr, VulkanInstance::GetDispatchLoader());

        for (int i = 0; i < shaders.size(); i++)
        {
            shaders[i]->VkShader = vkShaders[i];
        }

        return shaders;
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

    std::vector<vk::DescriptorSetLayoutBinding> Shader::GetDescriptorSetLayoutBindings() const
    {
        return DescriptorSetLayout->GetDescriptorSetLayoutBindings();
    }

    std::vector<vk::PushConstantRange> Shader::GetPushConstantRanges() const
    {
        return DescriptorSetLayout->GetPushConstantRanges();
    }

    const vk::DescriptorSetLayout &Shader::GetDescriptorSetLayout() const
    {
        return DescriptorSetLayout->GetDescriptorSetLayout();
    }

    uint32_t Shader::GetBindingFromIndex(uint32_t indexOfType, vk::DescriptorType type)
    {
        size_t index = 0;
        auto bindings = GetDescriptorSetLayoutBindings();
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

    uint32_t Shader::GetBindingFromIndex(uint32_t indexOfType, const std::vector<vk::DescriptorType> &types)
    {
        size_t index = 0;
        auto bindings = GetDescriptorSetLayoutBindings();
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

    ShaderInstance::ShaderInstance(const ShaderInstanceCreateInfo &createInfo)
    {
        if (createInfo.Shader == nullptr || createInfo.Shader->VkShader == nullptr)
        {
            throw std::runtime_error("Cannot create a ShaderInstance from a CreateInfo with an invalid shader");
        }

        Shader = createInfo.Shader;
        DescriptorPool = createInfo.DescriptorPool;

        CreateDescriptors();
    }

    ShaderInstance::ShaderInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool)
    {
        if (shader == nullptr || shader->VkShader == nullptr)
        {
            throw std::runtime_error("Cannot create a ShaderInstance with an invalid shader");
        }

        Shader = shader;
        DescriptorPool = descriptorPool;

        CreateDescriptors();
    }

    ShaderInstance::~ShaderInstance()
    {
        auto &device = Graphics::GetDevice();

        for (auto &sets : VkDescriptorSets)
        {
            if (!sets.empty())
            {
                DescriptorPool->FreeDescriptorSets(sets);
            }
            sets.clear();
        }

        if (VkPipelineLayout)
        {
            device.destroyPipelineLayout(VkPipelineLayout);
            VkPipelineLayout = nullptr;
        }
    }

    ShaderInstance::ShaderInstance(const ShaderInstance &other)
    {
        if (other.Shader == nullptr || other.Shader->VkShader == nullptr)
        {
            throw std::runtime_error("Cannot create a ShaderInstance from another ShaderInstance with an invalid Shader");
        }

        Shader = other.Shader;
        DescriptorPool = other.DescriptorPool;

        CreateDescriptors();
    }

    ShaderInstance &ShaderInstance::operator=(const ShaderInstance &other)
    {
        if (&other != this)
        {
            this->Shader = other.Shader;
            DescriptorPool = other.DescriptorPool;

            CreateDescriptors();
        }
        return *this;
    }

    void ShaderInstance::CreateDescriptors()
    {
        auto pushConstantRanges = Shader->GetPushConstantRanges();

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayouts(Shader->GetDescriptorSetLayout());
        pipelineLayoutCreateInfo.setPushConstantRanges(pushConstantRanges);
        VkPipelineLayout = Graphics::GetDevice().createPipelineLayout(pipelineLayoutCreateInfo);

        // Descriptor Sets
        for (auto &sets : VkDescriptorSets)
        {
            sets = DescriptorPool->AllocateDescriptorSets(Shader);
        }
    }

    vk::ShaderStageFlagBits ShaderInstance::GetShaderStage() const
    {
        return Shader->ShaderStage;
    }

    vk::ShaderStageFlags ShaderInstance::GetNextStage() const
    {
        return Shader->NextStage;
    }

    Spinner::Shader::Pointer ShaderInstance::GetShader() const
    {
        return Shader;
    }

    Spinner::DescriptorPool::Pointer ShaderInstance::GetDescriptorPool() const
    {
        return DescriptorPool;
    }

    std::vector<vk::DescriptorSet> ShaderInstance::GetDescriptorSets(uint32_t currentFrame) const
    {
        return VkDescriptorSets.at(currentFrame);
    }

    std::optional<vk::DescriptorType> ShaderInstance::GetDescriptorTypeOfBinding(uint32_t binding) const
    {
        auto layoutBindings = Shader->GetDescriptorSetLayoutBindings();

        for (const vk::DescriptorSetLayoutBinding &layoutBinding : layoutBindings)
        {
            if (layoutBinding.binding == binding)
            {
                return {layoutBinding.descriptorType};
            }
        }

        return {};
    }

    void ShaderInstance::UpdateDescriptorBuffer(uint32_t currentFrame, uint32_t binding, const std::shared_ptr<Buffer> &buffer) const
    {
        auto descriptorType = GetDescriptorTypeOfBinding(binding);
        if (!descriptorType.has_value())
        {
            throw std::runtime_error("Cannot get binding type from Shader's descriptor set layout bindings, binding #" + std::to_string(binding));
        }

        auto sets = GetDescriptorSets(currentFrame);

        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = buffer->VkBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = vk::WholeSize;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = sets.at(0); // WARNING: only one set is bound currently (check CreateDescriptors)
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType.value();
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    ShaderInstance::Pointer ShaderInstance::CreateInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool)
    {
        return std::make_shared<ShaderInstance>(shader, descriptorPool);
    }

    void ShaderInstance::UpdateDescriptorImage(uint32_t currentFrame, uint32_t binding, const std::shared_ptr<Texture> &texture, vk::ImageLayout imageLayout) const
    {
        assert(texture != nullptr);
        assert(texture->GetImage() != nullptr);
        assert(texture->GetImage()->GetMainImageView() != nullptr);
        assert(texture->GetSampler() != nullptr);

        UpdateDescriptorImage(currentFrame, binding, texture->GetImage()->GetMainImageView(), texture->GetSampler()->GetSampler());
    }

    void ShaderInstance::UpdateDescriptorImage(uint32_t currentFrame, uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout) const
    {
        auto descriptorType = GetDescriptorTypeOfBinding(binding);
        if (!descriptorType.has_value())
        {
            throw std::runtime_error("Cannot get binding type from Shader's descriptor set layout bindings, binding #" + std::to_string(binding));
        }

        auto sets = GetDescriptorSets(currentFrame);

        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = imageLayout;
        imageInfo.sampler = sampler;

        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = sets.at(0); // WARNING: only one set is bound currently (check CreateDescriptors)
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = descriptorType.value();
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;

        Graphics::GetDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    std::string GetShaderFileName(const std::string &shaderName, vk::ShaderStageFlagBits stage)
    {
        switch (stage)
        {
            default:
                throw std::runtime_error("Unhandled shader stage");
            case vk::ShaderStageFlagBits::eVertex:
                return shaderName + "_vert.spv";
            case vk::ShaderStageFlagBits::eFragment:
                return shaderName + "_frag.spv";
        }
    }
} // Spinner