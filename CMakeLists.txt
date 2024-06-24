cmake_minimum_required(VERSION 3.29)
project(Spinner)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan installation required")
endif ()
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED 3.4)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_library(Spinner OBJECT
        Spinner/Graphics.cpp
        Spinner/Graphics.hpp
        Spinner/Window.cpp
        Spinner/Window.hpp
        Spinner/App.cpp
        Spinner/App.hpp
        Spinner/VulkanInstance.cpp
        Spinner/VulkanInstance.hpp
        Spinner/SwapchainSupportDetails.hpp
        Spinner/QueueFamilyIndices.hpp
        Spinner/Swapchain.cpp
        Spinner/Swapchain.hpp
        Spinner/Callback.hpp
        Spinner/Object.hpp
        Spinner/Scene.cpp
        Spinner/Scene.hpp
        Spinner/SceneObject.cpp
        Spinner/SceneObject.hpp
        Spinner/GLM.hpp
        Spinner/Utilities.hpp
        Spinner/Components/Component.hpp
        Spinner/Components/MeshComponent.cpp
        Spinner/Components/MeshComponent.hpp
        Spinner/Buffer.cpp
        Spinner/Buffer.hpp
        Spinner/CommandBuffer.cpp
        Spinner/CommandBuffer.hpp
        Spinner/VertexDescription.hpp
        Spinner/Shader.cpp
        Spinner/Shader.hpp
        Spinner/MeshBuilder.cpp
        Spinner/MeshBuilder.hpp
        Spinner/VulkanUtilities.cpp
        Spinner/VulkanUtilities.hpp
        Spinner/MeshBuffer.cpp
        Spinner/MeshBuffer.hpp
        Spinner/MeshData/StaticMeshVertex.cpp
        Spinner/MeshData/StaticMeshVertex.hpp
        Spinner/DescriptorPool.cpp
        Spinner/DescriptorPool.hpp
        Spinner/Implementations.cpp
        Spinner/Extra/AlignedAllocator.hpp
        Spinner/DescriptorSetLayout.cpp
        Spinner/DescriptorSetLayout.hpp
)
target_link_libraries(Spinner PUBLIC Vulkan::Vulkan glfw glm::glm GPUOpen::VulkanMemoryAllocator)

add_executable(SpinnerTestApp
        main.cpp
)
target_link_libraries(SpinnerTestApp PRIVATE Spinner)