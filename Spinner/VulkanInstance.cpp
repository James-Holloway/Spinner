#include "VulkanInstance.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

namespace Spinner
{
    static std::vector<const char *> ValidationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };

    vk::Instance VulkanInstance::Instance;
    vk::DispatchLoaderDynamic VulkanInstance::DispatchLoader;
    vk::DebugUtilsMessengerEXT VulkanInstance::DebugMessenger;
    uint32_t VulkanInstance::VulkanVersion;
    bool VulkanInstance::Debug;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "VkVL ";
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            std::cerr << "[ERR]";
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            std::cerr << "[WRN]";
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            std::cerr << "[INF]";
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            std::cerr << "[VRB]";
        else
            std::cerr << "[UNK]";

        std::cerr << " " << pCallbackData->pMessage << '\n';

#ifndef NDEBUG
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cerr << std::endl;
            static volatile char dummy;
            (void) dummy; // Put breakpoint here to pause at debug callbacks and get the stack trace
        }
#endif

        return vk::False;
    }

    vk::Instance &VulkanInstance::GetInstance()
    {
        return Instance;
    }

    vk::DispatchLoaderDynamic &VulkanInstance::GetDispatchLoader()
    {
        return DispatchLoader;
    }

    void VulkanInstance::CreateInstance(const std::string &appName, unsigned int vulkanVersion, unsigned int appVersion, bool debug, const std::vector<const char *> &appRequiredExtensions, std::vector<const char *> layersToEnable)
    {
        if (Instance)
        {
            throw std::runtime_error("Cannot create another vulkan instance as one already exists");
        }
        Debug = debug;
        VulkanVersion = vulkanVersion;

        vk::ApplicationInfo appInfo;
        const char *cAppName = appName.c_str();
        appInfo.pApplicationName = cAppName;
        appInfo.applicationVersion = appVersion;
        appInfo.pEngineName = "Spinner";
        appInfo.engineVersion = vk::makeApiVersion(0, 1, 0, 0);
        appInfo.apiVersion = VulkanVersion;

        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char *> requiredExtensions;
        {
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            for (uint32_t i = 0; i < glfwExtensionCount; i++)
            {
                requiredExtensions.push_back(glfwExtensions[i]);
            }

            for (auto &extension : appRequiredExtensions)
            {
                requiredExtensions.push_back(extension);
            }

            if (debug)
            {
                requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
            }
        }

        // Push the (potentially) required layers required by Spinner
        layersToEnable.push_back("VK_LAYER_KHRONOS_shader_object");

        if (debug)
        {
            for (auto &layer : ValidationLayers)
            {
                layersToEnable.push_back(layer);
            }
        }

        createInfo.setPEnabledExtensionNames(requiredExtensions);
        createInfo.setPEnabledLayerNames(layersToEnable);

        Instance = vk::createInstance(createInfo);

        auto getInstanceProcAddr = (PFN_vkGetInstanceProcAddr) Instance.getProcAddr("vkGetInstanceProcAddr");
        DispatchLoader = vk::DispatchLoaderDynamic(Instance, getInstanceProcAddr);

        if (debug)
        {
            vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
            debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            debugCreateInfo.pfnUserCallback = &DebugCallback;
            debugCreateInfo.pUserData = nullptr;

            DebugMessenger = Instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, DispatchLoader);
        }
    }

    void VulkanInstance::DestroyInstance()
    {
        if (!Instance)
        {
            throw std::runtime_error("Cannot destroy an instance that has not been created yet");
        }

        if (IsDebug())
        {
            Instance.destroyDebugUtilsMessengerEXT(DebugMessenger, nullptr, DispatchLoader);
        }

        Instance.destroy();
    }

    bool VulkanInstance::HasInstanceBeenCreated()
    {
        return (bool) Instance;
    }

    bool VulkanInstance::IsDebug()
    {
        return Debug;
    }

    uint32_t VulkanInstance::GetVulkanVersion()
    {
        return VulkanVersion;
    }

    void VulkanInstance::ReinitializeDispatcher(const vk::Device &device)
    {
        auto getInstanceProcAddr = (PFN_vkGetInstanceProcAddr) Instance.getProcAddr("vkGetInstanceProcAddr");
        auto getDeviceProcAddr = (PFN_vkGetDeviceProcAddr) device.getProcAddr("vkGetDeviceProcAddr");
        DispatchLoader.init(Instance, getInstanceProcAddr, device, getDeviceProcAddr);
    }

} // Spinner