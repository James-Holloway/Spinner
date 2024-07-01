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

    class App;

    class Input
    {
        friend class App;

    public:
        using Pointer = std::shared_ptr<Input>;

        Input() = default;
        ~Input() = default;

    protected:
        constexpr static const uint8_t KeyStateNone = 0;
        constexpr static const uint8_t KeyStatePressed = 1;
        constexpr static const uint8_t KeyStateReleased = 2;
        constexpr static const uint8_t KeyStateRepeated = 2;

        std::array<bool, LastKeyEnumValue> PressedKeys{};
        std::array<uint8_t, LastKeyEnumValue> KeyStateSinceLastFrame{};

        void ResetFrameKeyState();

    public:
        Callback<Key, bool> KeyCallback;

    public:
        // Used internally
        void SendKeyEvent(int key, int scancode, int action, int mods);
        // Used internally
        void SendMouseButtonEvent(int button, int action, int mods);

        // If the key is currently known to be pressed
        bool GetKeyDown(Key key);
        // If the key is currently known to be not pressed
        bool GetKeyUp(Key key);

        // Returns whether the key was just pressed this last frame
        bool GetKeyPressed(Key key);
        // Returns whether the key was just released this last frame
        bool GetKeyReleased(Key key);
        // Returns whether the key was just repeated this last frame
        bool GetKeyRepeated(Key key);
        // Returns whether the key was just pressed or repeated this last frame
        bool GetKeyPressedOrRepeated(Key key);
    };

} // Spinner

#endif //SPINNER_INPUT_HPP
