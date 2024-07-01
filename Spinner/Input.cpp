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

    void Input::ResetFrameKeyState()
    {
        std::memset(KeyStateSinceLastFrame.data(), KeyStateNone, KeyStateSinceLastFrame.size() * sizeof(uint8_t));
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

    bool Input::GetKeyDown(Key key)
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return PressedKeys.at(GetGLFWKey(key));
    }

    bool Input::GetKeyUp(Key key)
    {
        return !GetKeyDown(key);
    }

    bool Input::GetKeyPressed(Key key)
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStatePressed;
    }

    bool Input::GetKeyReleased(Key key)
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStateReleased;
    }

    bool Input::GetKeyRepeated(Key key)
    {
        assert(GetGLFWKey(key) <= LastKeyEnumValue);
        return KeyStateSinceLastFrame.at(GetGLFWKey(key)) == KeyStateReleased;
    }

    bool Input::GetKeyPressedOrRepeated(Key key)
    {
        return GetKeyPressed(key) || GetKeyRepeated(key);
    }


} // Spinner