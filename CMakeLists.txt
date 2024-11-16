cmake_minimum_required(VERSION 3.26)
project(Spinner)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan installation required")
endif ()
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED 3.4)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include(cmake/Shaders.cmake)
include(cmake/Assets.cmake)

add_subdirectory(ThirdParty/tinygltf)

add_library(imgui STATIC
        ThirdParty/imgui/imgui.cpp
        ThirdParty/imgui/imgui_demo.cpp
        ThirdParty/imgui/imgui_draw.cpp
        ThirdParty/imgui/imgui_tables.cpp
        ThirdParty/imgui/imgui_widgets.cpp
        ThirdParty/imgui/backends/imgui_impl_glfw.cpp
        ThirdParty/imgui/backends/imgui_impl_vulkan.cpp
        ThirdParty/imgui/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC ThirdParty/imgui)

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
        Spinner/Components/Component.cpp
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
        Spinner/Constants.hpp
        Spinner/Components/CameraComponent.cpp
        Spinner/Components/CameraComponent.hpp
        Spinner/ImGuiInstance.cpp
        Spinner/ImGuiInstance.hpp
        Spinner/Image.cpp
        Spinner/Image.hpp
        Spinner/Material.cpp
        Spinner/Material.hpp
        Spinner/Texture.cpp
        Spinner/Texture.hpp
        Spinner/Sampler.cpp
        Spinner/Sampler.hpp
        Spinner/Light.cpp
        Spinner/Light.hpp
        Spinner/Components/LightComponent.cpp
        Spinner/Components/LightComponent.hpp
        Spinner/Lighting.cpp
        Spinner/Lighting.hpp
        Spinner/Components/Components.hpp
        Spinner/Input.cpp
        Spinner/Input.hpp
        Spinner/Key.hpp
        Spinner/DrawManager.cpp
        Spinner/DrawManager.hpp
        Spinner/DrawCommand.cpp
        Spinner/DrawCommand.hpp
        Spinner/Passes.hpp
        Spinner/ScopedTimer.hpp
)
target_link_libraries(Spinner PUBLIC Vulkan::Vulkan glfw glm::glm GPUOpen::VulkanMemoryAllocator tinygltf imgui)

add_executable(SpinnerApp
        SpinnerApp/main.cpp
        SpinnerApp/SpinnerApp.cpp
        SpinnerApp/SpinnerApp.hpp
)
target_link_libraries(SpinnerApp PRIVATE Spinner)
target_include_directories(SpinnerApp PUBLIC "${CMAKE_SOURCE_DIR}")

# Shaders (creates SpinnerShaders target)
compile_shaders(Spinner
        Shaders/staticmesh.vert
        Shaders/staticmesh.frag
        Shaders/staticshadow.vert
        Shaders/staticshadow.frag
)

# Asset Files (note: cannot be applied to OBJECT library)
set_assets(SpinnerApp SYMLINK FILES
        Assets/Models
        Assets/Textures/DuckCM.png
)