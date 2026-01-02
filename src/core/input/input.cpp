#include "core/input.h"
#include "core/hardware_detection.h"
#include <Arduino.h>

namespace NightStrike {
namespace Core {

Input& Input::getInstance() {
    static Input instance;
    return instance;
}

Error Input::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize hardware buttons based on board
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: Button A (GPIO 37), Button B (GPIO 39)
    pinMode(37, INPUT_PULLUP);
    pinMode(39, INPUT_PULLUP);
    Serial.println("[Input] M5StickC PLUS2 buttons initialized (A: GPIO37, B: GPIO39)");
#else
    // Generic ESP32 - use serial input as fallback
    Serial.println("[Input] Using serial input (no hardware buttons detected)");
#endif

    _initialized = true;
    return Error(ErrorCode::SUCCESS);
}

Error Input::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    _buttonCallback = nullptr;
    _touchCallback = nullptr;
    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

Error Input::registerButtonCallback(ButtonCallback callback) {
    _buttonCallback = callback;
    return Error(ErrorCode::SUCCESS);
}

bool Input::isButtonPressed(Button button) const {
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2 buttons
    switch (button) {
        case Button::SELECT:
            return digitalRead(37) == LOW;  // Button A pressed
        case Button::BACK:
            return digitalRead(39) == LOW;   // Button B pressed
        default:
            return false;
    }
#else
    // Generic ESP32 - no hardware buttons
    return false;
#endif
}

Error Input::registerTouchCallback(TouchCallback callback) {
    _touchCallback = callback;
    return Error(ErrorCode::SUCCESS);
}

bool Input::isTouchPressed() const {
    return _touchPoint.pressed;
}

void Input::update() {
    if (!_initialized) {
        return;
    }

#ifdef M5STICKC_PLUS2
    // Read M5StickC PLUS2 hardware buttons
    static unsigned long lastDebounceTime = 0;
    static Button lastButtonState = Button::NONE;
    const unsigned long debounceDelay = 50;
    
    Button currentState = Button::NONE;
    
    // Check Button A (SELECT) - GPIO 37
    if (digitalRead(37) == LOW) {
        currentState = Button::SELECT;
    }
    // Check Button B (BACK) - GPIO 39
    else if (digitalRead(39) == LOW) {
        currentState = Button::BACK;
    }
    
    // Debounce
    if (currentState != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentState != Button::NONE && currentState != _currentButtonState) {
            _lastButton = currentState;
            _currentButtonState = currentState;
            if (_buttonCallback) {
                _buttonCallback(currentState, EventType::PRESS);
            }
        } else if (currentState == Button::NONE && _currentButtonState != Button::NONE) {
            if (_buttonCallback) {
                _buttonCallback(_currentButtonState, EventType::RELEASE);
            }
            _currentButtonState = Button::NONE;
        }
    }
    
    lastButtonState = currentState;
#else
    // Fallback: check serial for keyboard input
    if (Serial.available()) {
        char c = Serial.read();
        // Map serial input to buttons for testing
        Button btn = Button::NONE;
        switch (c) {
            case 'w': case 'W': btn = Button::UP; break;
            case 's': case 'S': btn = Button::DOWN; break;
            case 'a': case 'A': btn = Button::LEFT; break;
            case 'd': case 'D': btn = Button::RIGHT; break;
            case ' ': case '\r': case '\n': btn = Button::SELECT; break;
            case 'b': case 'B': btn = Button::BACK; break;
            case 'm': case 'M': btn = Button::MENU; break;
        }

        if (btn != Button::NONE && _buttonCallback) {
            _lastButton = btn;
            _buttonCallback(btn, EventType::PRESS);
        }
    }
#endif
}

Error Input::processKeyboardInput(const char* input) {
    if (!input) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

    // Process keyboard input (for BadUSB-like functionality)
    Serial.printf("[Input] Keyboard input: %s\n", input);
    return Error(ErrorCode::SUCCESS);
}

} // namespace Core
} // namespace NightStrike

