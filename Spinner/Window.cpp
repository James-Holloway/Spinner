#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include "VulkanInstance.hpp"

namespace Spinner
{
    static void FrameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        void *userPointer = glfwGetWindowUserPointer(window);
        if (userPointer != nullptr)
        {
            reinterpret_cast<Window *>(userPointer)->SetFrameBufferNeedsResize(true);
        }
    }

    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        void *userPointer = glfwGetWindowUserPointer(window);
        if (userPointer != nullptr)
        {
            auto input = reinterpret_cast<Window *>(userPointer)->GetInput();
            if (input != nullptr)
            {
                input->SendKeyEvent(key, scancode, action, mods);
            }
        }
    }

    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
    {
        void *userPointer = glfwGetWindowUserPointer(window);
        if (userPointer != nullptr)
        {
            auto input = reinterpret_cast<Window *>(userPointer)->GetInput();
            if (input != nullptr)
            {
                input->SendMouseButtonEvent(button, action, mods);
            }
        }
    }

    void Window::Create(int width, int height, const std::string &title)
    {
        if (GLFWWindow != nullptr)
        {
            throw std::runtime_error("Window already created");
        }

        Width = width;
        Height = height;
        Title = title;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (GLFWWindow == nullptr)
        {
            throw std::runtime_error("Window could not be created");
        }

        Input = std::make_shared<Spinner::Input>();

        glfwSetWindowUserPointer(GLFWWindow, this);
        glfwSetFramebufferSizeCallback(GLFWWindow, &FrameBufferResizeCallback);
        glfwSetKeyCallback(GLFWWindow, &KeyCallback);
        glfwSetMouseButtonCallback(GLFWWindow, &MouseButtonCallback);
    }

    void Window::Destroy()
    {
        Input.reset();

        if (GLFWWindow == nullptr)
        {
            throw std::runtime_error("Cannot destroy window as it has not been created");
        }

        glfwSetWindowUserPointer(GLFWWindow, nullptr);
        glfwDestroyWindow(GLFWWindow);

        Width = 0;
        Height = 0;
        GLFWWindow = nullptr;
    }

    bool Window::IsValid() const
    {
        return GLFWWindow != nullptr;
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(GLFWWindow);
    }

    void Window::CreateSurface()
    {
        if (Surface)
        {
            throw std::runtime_error("Surface already exists for window");
        }
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(VulkanInstance::GetInstance(), GLFWWindow, nullptr, &surface);
        Surface = vk::SurfaceKHR{surface};
    }

    void Window::DestroySurface()
    {
        if (!Surface)
        {
            throw std::runtime_error("Surface does not exist");
        }
        VulkanInstance::GetInstance().destroySurfaceKHR(Surface);
    }

    vk::SurfaceKHR Window::GetSurface() const
    {
        return Surface;
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    bool Window::DoesFrameBufferNeedsResize() const
    {
        return FrameBufferNeedsResize;
    }

    void Window::SetFrameBufferNeedsResize(bool dirty)
    {
        FrameBufferNeedsResize = dirty;
    }

    GLFWwindow *Window::GetWindow() const
    {
        return GLFWWindow;
    }

    Spinner::Input::Pointer Window::GetInput() const
    {
        return Input;
    }


} // Spinner