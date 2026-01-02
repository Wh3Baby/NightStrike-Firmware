#pragma once

#include "errors.h"
#include <cstdint>
#include <functional>

namespace NightStrike {
namespace Core {

/**
 * @brief Input handling system
 * Supports buttons, touch, keyboard, and encoder
 */
class Input {
public:
    enum class Button {
        NONE = 0,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        SELECT,
        BACK,
        MENU
    };

    enum class EventType {
        PRESS,
        RELEASE,
        LONG_PRESS,
        DOUBLE_PRESS
    };

    struct TouchPoint {
        int16_t x, y;
        bool pressed;
    };

    using ButtonCallback = std::function<void(Button, EventType)>;
    using TouchCallback = std::function<void(const TouchPoint&)>;

    static Input& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // Button handling
    Error registerButtonCallback(ButtonCallback callback);
    bool isButtonPressed(Button button) const;
    Button getLastPressedButton() const { return _lastButton; }

    // Touch handling
    Error registerTouchCallback(TouchCallback callback);
    bool isTouchPressed() const;
    TouchPoint getTouchPoint() const { return _touchPoint; }

    // Update (call in loop)
    void update();

    // Keyboard input (for devices with keyboard)
    Error processKeyboardInput(const char* input);

private:
    Input() = default;
    ~Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    bool _initialized = false;
    Button _lastButton = Button::NONE;
    TouchPoint _touchPoint = {0, 0, false};
    ButtonCallback _buttonCallback = nullptr;
    TouchCallback _touchCallback = nullptr;

    unsigned long _lastButtonTime = 0;
    Button _lastButtonState = Button::NONE;
    mutable Button _currentButtonState = Button::NONE;  // For debouncing
};

} // namespace Core
} // namespace NightStrike

