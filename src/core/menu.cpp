#include "core/menu.h"
#include "core/display.h"
#include "core/input.h"
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
    auto& input = Input::getInstance();
    input.registerButtonCallback([this](Input::Button btn, Input::EventType type) {
        if (type == Input::EventType::PRESS && _visible) {
            switch (btn) {
                case Input::Button::UP:
                    selectPrevious();
                    break;
                case Input::Button::DOWN:
                    selectNext();
                    break;
                case Input::Button::SELECT:
                    if (_selectedIndex < _items.size() && _items[_selectedIndex].enabled) {
                        _items[_selectedIndex].action();
                    }
                    break;
                case Input::Button::BACK:
                    // BACK button - call setupMainMenu to return to main menu
                    // This is handled by menu handlers, but we need a way to trigger it
                    // For now, we'll let the handlers manage this via "Back" menu items
                    break;
                default:
                    break;
            }
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

