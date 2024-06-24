#ifndef SPINNER_VULKANINSTANCE_HPP
#define SPINNER_VULKANINSTANCE_HPP

#include <vulkan/vulkan.hpp>


namespace Spinner
{

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class VulkanInstance
    {
    protected:
        static vk::Instance Instance;
        static vk::DispatchLoaderDynamic DispatchLoader;
        static vk::DebugUtilsMessengerEXT DebugMessenger;
        static uint32_t VulkanVersion;
        static bool Debug;

    public:
        static vk::Instance &GetInstance();
        static vk::DispatchLoaderDynamic &GetDispatchLoader();
        static void CreateInstance(const std::string &appName, unsigned int vulkanVersion, unsigned int appVersion, bool debug = false, const std::vector<const char *> &appRequiredExtensions = {}, std::vector<const char *> layersToEnable = {});
        static void DestroyInstance();
        static bool HasInstanceBeenCreated();
        static bool IsDebug();
        static uint32_t GetVulkanVersion();
        static void ReinitializeDispatcher(const vk::Device& device);
    };

} // Spinner

#endif //SPINNER_VULKANINSTANCE_HPP
