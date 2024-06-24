#include "Shader.hpp"

#include "Utilities.hpp"
#include "Graphics.hpp"

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

    const vk::DescriptorSetLayout& Shader::GetDescriptorSetLayout() const
    {
        return DescriptorSetLayout->GetDescriptorSetLayout();
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

        if (!VkDescriptorSets.empty())
        {
            DescriptorPool->FreeDescriptorSets(VkDescriptorSets);
            VkDescriptorSets.clear();
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

    vk::ShaderStageFlagBits ShaderInstance::GetShaderStage() const
    {
        return Shader->ShaderStage;
    }

    vk::ShaderStageFlags ShaderInstance::GetNextStage() const
    {
        return Shader->NextStage;
    }

    void ShaderInstance::CreateDescriptors()
    {
        auto &device = Graphics::GetDevice();

        auto pushConstantRanges = Shader->GetPushConstantRanges();

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayouts(Shader->GetDescriptorSetLayout());
        pipelineLayoutCreateInfo.setPushConstantRanges(pushConstantRanges);
        VkPipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

        // Descriptor Sets
        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.descriptorPool = DescriptorPool->VkDescriptorPool;
        allocateInfo.setSetLayouts(Shader->GetDescriptorSetLayout());
        VkDescriptorSets = device.allocateDescriptorSets(allocateInfo);

        DescriptorPool->AllocateDescriptorSets(Shader);
        DescriptorPool->AllocateDescriptorSets(Shader->GetDescriptorSetLayout());
    }

    ShaderInstance::Pointer ShaderInstance::CreateInstance(const Shader::Pointer &shader, const DescriptorPool::Pointer &descriptorPool)
    {
        return std::make_shared<ShaderInstance>(shader, descriptorPool);
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