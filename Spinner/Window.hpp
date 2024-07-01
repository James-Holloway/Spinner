#ifndef SPINNER_WINDOW_HPP
#define SPINNER_WINDOW_HPP

#include <string>
#include <vulkan/vulkan.hpp>
#include "Input.hpp"

struct GLFWwindow;

namespace Spinner
{
    class Graphics;

    class Window
    {
        friend class Graphics;

    public:
        Window() = default;
        ~Window() = default;

    protected:
        GLFWwindow *GLFWWindow = nullptr;

        std::string Title;
        int Width = 0, Height = 0;
        vk::SurfaceKHR Surface;
        bool FrameBufferNeedsResize = false;
        Spinner::Input::Pointer Input;

    public:
        void Create(int width, int height, const std::string &title);
        void Destroy();
        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] bool ShouldClose() const;
        [[nodiscard]] vk::SurfaceKHR GetSurface() const;
        [[nodiscard]] GLFWwindow *GetWindow() const;
        void CreateSurface();
        void DestroySurface();
        static void PollEvents();
        [[nodiscard]] bool DoesFrameBufferNeedsResize() const;
        void SetFrameBufferNeedsResize(bool dirty);
        [[nodiscard]] Spinner::Input::Pointer GetInput() const;
    };

} // Spinner

#endif //SPINNER_WINDOW_HPP
