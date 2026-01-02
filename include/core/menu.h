#pragma once

#include "errors.h"
#include "display.h"
#include "input.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Core {

/**
 * @brief Menu system for navigation
 */
class Menu {
public:
    struct MenuItem {
        std::string label;
        std::string icon;  // Icon name or path
        std::function<void()> action;
        bool enabled = true;

        MenuItem(const std::string& lbl, std::function<void()> act)
            : label(lbl), action(act) {}
    };

    static Menu& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // Menu management
    Error addItem(const MenuItem& item);
    Error removeItem(const std::string& label);
    void clear();

    // Navigation
    Error show();
    Error hide();
    void update();

    // Selection
    void selectNext();
    void selectPrevious();
    void selectItem(size_t index);
    size_t getSelectedIndex() const { return _selectedIndex; }

    // Callbacks
    using RenderCallback = std::function<void(const MenuItem&, bool selected)>;
    Error setRenderCallback(RenderCallback callback);

private:
    Menu() = default;
    ~Menu() = default;
    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;

    bool _initialized = false;
    bool _visible = false;
    std::vector<MenuItem> _items;
    size_t _selectedIndex = 0;
    RenderCallback _renderCallback = nullptr;

    void render();
    void handleInput();
};

} // namespace Core
} // namespace NightStrike

