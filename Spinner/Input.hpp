#ifndef SPINNER_INPUT_HPP
#define SPINNER_INPUT_HPP

#include <memory>
#include "Key.hpp"
#include "Callback.hpp"

namespace Spinner
{
    const char *GetKeyName(Key key);
    int GetGLFWKey(Key key);
    Key GetSpinnerKey(int glfwKey);

    class Input
    {
    public:
        using Pointer = std::shared_ptr<Input>;

        Input() = default;
        ~Input() = default;

    protected:
        constexpr static const uint8_t KeyStateNone = 0;
        constexpr static const uint8_t KeyStatePressed = 1;
        constexpr static const uint8_t KeyStateReleased = 2;
        constexpr static const uint8_t KeyStateRepeated = 3;

        std::array<bool, LastKeyEnumValue> PressedKeys{};
        std::array<uint8_t, LastKeyEnumValue> KeyStateSinceLastFrame{};
        double CursorPositionX = 0.0;
        double CursorPositionY = 0.0;
        double LastFrameCursorPositionX = 0.0;
        double LastFrameCursorPositionY = 0.0;
        double ScrollOffsetX = 0.0;
        double ScrollOffsetY = 0.0;

    public:
        Callback<Key, bool> KeyCallback;
        Callback<double, double> CursorPositionCallback;

    public:
        // Use when resetting the state for a new frame, used already in App
        void ResetFrameState();
        // Used internally
        void SendKeyEvent(int key, int scancode, int action, int mods);
        // Used internally
        void SendMouseButtonEvent(int button, int action, int mods);
        // Used internally
        void SetCursorPosition(double xPos, double yPos);
        // Used internally
        void SetScrollOffset(double xOffset, double yOffset);

        // If the key is currently known to be pressed
        [[nodiscard]] bool GetKeyDown(Key key) const;
        // If the key is currently known to be not pressed
        [[nodiscard]] bool GetKeyUp(Key key) const;

        // Returns whether the key was just pressed this last frame
        [[nodiscard]] bool GetKeyPressed(Key key) const;
        // Returns whether the key was just released this last frame
        [[nodiscard]] bool GetKeyReleased(Key key) const;
        // Returns whether the key was just repeated this last frame
        [[nodiscard]] bool GetKeyRepeated(Key key) const;
        // Returns whether the key was just pressed or repeated this last frame
        [[nodiscard]] bool GetKeyPressedOrRepeated(Key key) const;

        // Get the cursor position relative to the top left of the window
        void GetCursorPosition(double &xPos, double &yPos) const;
        // Get the cursor position relative to the top left of the window
        void GetCursorPosition(int &xPos, int &yPos) const;
        // Get the change in cursor position
        void GetCursorDeltaPosition(double &deltaX, double &deltaY) const;
        // Get the change in cursor position
        void GetCursorDeltaPosition(int &deltaX, int &deltaY) const;
        // Get the horizontal and vertical scroll offsets this past frame
        void GetScrollOffsetDeltas(double &xOffset, double &yOffset) const;
        // Get the vertical scroll offset this past frame
        [[nodiscard]] double GetVerticalScrollDelta() const;
        // Get the horizontal scroll offset this past frame
        [[nodiscard]] double GetHorizontalScrollDelta() const;
    };
} // Spinner

#endif //SPINNER_INPUT_HPP
