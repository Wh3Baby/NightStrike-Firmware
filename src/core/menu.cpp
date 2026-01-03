#include "core/menu.h"
#include "core/display.h"
#include "core/input.h"
#include "core/power_management.h"
#include <Arduino.h>
#include <algorithm>

namespace NightStrike {
namespace Core {

Menu& Menu::getInstance() {
    static Menu instance;
    return instance;
}

Error Menu::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    // Register input callbacks
    // For M5StickC PLUS2: Button A (SELECT) = navigate/select, Button B (BACK) = back
    // Navigation: Short press A = next item, Double press A = select, Button B = back
    auto& input = Input::getInstance();
    static unsigned long lastSelectPress = 0;
    
    input.registerButtonCallback([this](Input::Button btn, Input::EventType type) {
        if (!_visible) return;
        
        switch (btn) {
            case Input::Button::UP:
                selectPrevious();
                break;
            case Input::Button::DOWN:
                selectNext();
                break;
            case Input::Button::SELECT:
                if (type == Input::EventType::PRESS) {
                    unsigned long now = millis();
                    // Double-click detection (within 400ms) = select
                    if (lastSelectPress > 0 && (now - lastSelectPress) < 400) {
                        // Double-click = select item
                        if (_selectedIndex < _items.size() && _items[_selectedIndex].enabled) {
                            _items[_selectedIndex].action();
                        }
                        lastSelectPress = 0; // Reset
                    } else {
                        // Single click = next item
                        selectNext();
                        lastSelectPress = now;
                    }
                } else if (type == Input::EventType::LONG_PRESS) {
                    // Long press = select item
                    if (_selectedIndex < _items.size() && _items[_selectedIndex].enabled) {
                        _items[_selectedIndex].action();
                    }
                }
                break;
            case Input::Button::BACK:
                // BACK button handling is done via "Back" menu items in each menu
                // This allows menu handlers to manage navigation properly
                break;
            default:
                break;
        }
    });

    _initialized = true;
    return Error(ErrorCode::SUCCESS);
}

Error Menu::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    hide();
    clear();
    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

Error Menu::addItem(const MenuItem& item) {
    _items.push_back(item);
    return Error(ErrorCode::SUCCESS);
}

Error Menu::removeItem(const std::string& label) {
    auto it = std::remove_if(_items.begin(), _items.end(),
        [&label](const MenuItem& item) { return item.label == label; });

    if (it != _items.end()) {
        _items.erase(it, _items.end());
        if (_selectedIndex >= _items.size()) {
            _selectedIndex = _items.size() > 0 ? _items.size() - 1 : 0;
        }
        return Error(ErrorCode::SUCCESS);
    }

    return Error(ErrorCode::OPERATION_FAILED);
}

void Menu::clear() {
    _items.clear();
    _selectedIndex = 0;
}

Error Menu::show() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    _visible = true;
    render();
    return Error(ErrorCode::SUCCESS);
}

Error Menu::hide() {
    _visible = false;
    auto& display = Display::getInstance();
    display.clear();
    return Error(ErrorCode::SUCCESS);
}

void Menu::update() {
    if (!_visible || !_initialized) {
        return;
    }

    handleInput();
}

void Menu::selectNext() {
    if (_items.empty()) {
        return;
    }

    _selectedIndex = (_selectedIndex + 1) % _items.size();
    render();
}

void Menu::selectPrevious() {
    if (_items.empty()) {
        return;
    }

    _selectedIndex = (_selectedIndex == 0) ? _items.size() - 1 : _selectedIndex - 1;
    render();
}

void Menu::selectItem(size_t index) {
    if (index < _items.size()) {
        _selectedIndex = index;
        render();
    }
}

Error Menu::setRenderCallback(RenderCallback callback) {
    _renderCallback = callback;
    return Error(ErrorCode::SUCCESS);
}

void Menu::render() {
    if (_items.empty()) {
        return;
    }

    auto& display = Display::getInstance();
    display.clear();

    // Draw battery indicator in top-right corner
    auto& power = PowerManagement::getInstance();
    if (power.isInitialized()) {
        int batteryLevel = power.getBatteryLevel();
        bool isCharging = power.isCharging();
        if (batteryLevel >= 0) {
            // Position: top-right corner (240x135 display)
            // Battery icon is 24px wide + 2px tip + ~20px text = ~46px total
            // Position at x = 240 - 46 = 194, y = 2
            display.drawBatteryIndicator(Display::Point(194, 2), batteryLevel, isCharging);
        }
    }

    if (_renderCallback) {
        // Use custom renderer
        for (size_t i = 0; i < _items.size(); ++i) {
            _renderCallback(_items[i], i == _selectedIndex);
        }
    } else {
        // Default renderer
        display.setTextColor(Display::Color::White(), Display::Color::Black());
        display.setTextSize(1);

        int y = 10;
        for (size_t i = 0; i < _items.size() && i < 10; ++i) {
            Display::Color fg = (i == _selectedIndex) ? Display::Color::Green() : Display::Color::White();
            Display::Color bg = (i == _selectedIndex) ? Display::Color::Black() : Display::Color::Black();

            display.setTextColor(fg, bg);
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%s %s",
                (i == _selectedIndex) ? ">" : " ",
                _items[i].label.c_str());
            display.drawText(Display::Point(5, y), buffer);
            y += 15;
        }
    }
}

void Menu::handleInput() {
    // Input handling is done via callbacks
}

} // namespace Core
} // namespace NightStrike

