#include "Input.hpp"

#include <GLFW/glfw3.h>
#include <cassert>
#include <cstring>

namespace Spinner
{
    const char *GetKeyName(Key key)
    {
        return glfwGetKeyName(static_cast<int>(key), 0);
    }

    int GetGLFWKey(Key key)
    {
        return static_cast<int>(key);
    }

    Key GetSpinnerKey(int glfwKey)
    {
        return static_cast<Key>(glfwKey);
    }

    void Input::ResetFrameState()
    {
        std::memset(KeyStateSinceLastFrame.data(), KeyStateNone, KeyStateSinceLastFrame.size() * sizeof(uint8_t));

        LastFrameCursorPositionX = CursorPositionX;
        LastFrameCursorPositionY = CursorPositionY;

        ScrollOffsetX = 0.0;
        ScrollOffsetY = 0.0;
    }

    void Input::SendKeyEvent(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_UNKNOWN || key > GLFW_KEY_LAST)
        {
            return;
        }

        Key spinnerKey = GetSpinnerKey(key);
        bool down = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        PressedKeys.at(key) = down;

        KeyCallback.Run(spinnerKey, down);

        switch (action)
        {
            case GLFW_PRESS:
                KeyStateSinceLastFrame.at(key) = KeyStatePressed;
                break;
            case GLFW_RELEASE:
                KeyStateSinceLastFrame.at(key) = KeyStateReleased;
                break;
            case GLFW_REPEAT:
                KeyStateSinceLastFrame.at(key) = KeyStateRepeated;
                break;
            default:
                break;
        }
    }

    void Input::SendMouseButtonEvent(int button, int action, int mods)
    {
        SendKeyEvent(button, 0, action, mods);
    }

    void Input::SetCursorPosition(double xPos, double yPos)
    {
        CursorPositionX = xPos;
        CursorPositionY = yPos;

        CursorPositionCallback.Run(CursorPositionX, CursorPositionY);
    }

    void Input::SetScrollOffset(double xOffset, double yOffset)
    {
        ScrollOffsetX += xOffset;
        ScrollOffsetY += yOffset;
    }

    bool Input::GetKeyDown(Key key) const
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return PressedKeys.at(GetGLFWKey(key));
    }

    bool Input::GetKeyUp(Key key) const
    {
        return !GetKeyDown(key);
    }

    bool Input::GetKeyPressed(Key key) const
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStatePressed;
    }

    bool Input::GetKeyReleased(Key key) const
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStateReleased;
    }

    bool Input::GetKeyRepeated(Key key) const
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStateReleased;
    }

    bool Input::GetKeyPressedOrRepeated(Key key) const
    {
        return GetKeyPressed(key) || GetKeyRepeated(key);
    }

    void Input::GetCursorPosition(double &xPos, double &yPos) const
    {
        xPos = CursorPositionX;
        yPos = CursorPositionY;
    }

    void Input::GetCursorDeltaPosition(double &deltaX, double &deltaY) const
    {
        deltaX = CursorPositionX - LastFrameCursorPositionX;
        deltaY = CursorPositionY - LastFrameCursorPositionY;
    }

    void Input::GetCursorPosition(int &xPos, int &yPos) const
    {
        xPos = static_cast<int>(CursorPositionX);
        yPos = static_cast<int>(CursorPositionY);
    }

    void Input::GetCursorDeltaPosition(int &deltaX, int &deltaY) const
    {
        deltaX = static_cast<int>(CursorPositionX - LastFrameCursorPositionX);
        deltaY = static_cast<int>(CursorPositionY - LastFrameCursorPositionY);
    }

    void Input::GetScrollOffsetDeltas(double &xOffset, double &yOffset) const
    {
        xOffset = ScrollOffsetX;
        yOffset = ScrollOffsetY;
    }

    double Input::GetVerticalScrollDelta() const
    {
        return ScrollOffsetY;
    }

    double Input::GetHorizontalScrollDelta() const
    {
        return ScrollOffsetX;
    }
} // Spinner