#pragma once

#include "errors.h"
#include <cstdint>

namespace NightStrike {
namespace Core {

/**
 * @brief Display abstraction layer
 * Supports TFT displays and serial output
 */
class Display {
public:
    struct Color {
        uint16_t value;
        Color(uint16_t v) : value(v) {}
        static Color Black() { return Color(0x0000); }
        static Color White() { return Color(0xFFFF); }
        static Color Red() { return Color(0xF800); }
        static Color Green() { return Color(0x07E0); }
        static Color Blue() { return Color(0x001F); }
    };

    struct Point {
        int16_t x, y;
        Point(int16_t x = 0, int16_t y = 0) : x(x), y(y) {}
    };

    struct Size {
        uint16_t width, height;
        Size(uint16_t w = 0, uint16_t h = 0) : width(w), height(h) {}
    };

    static Display& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // Display control
    Error setBrightness(uint8_t brightness);
    Error setRotation(uint8_t rotation);
    Error fillScreen(Color color);
    Error clear();

    // Drawing primitives
    Error drawPixel(Point pos, Color color);
    Error drawLine(Point start, Point end, Color color);
    Error drawRect(Point pos, Size size, Color color, bool filled = false);
    Error drawCircle(Point center, uint16_t radius, Color color, bool filled = false);

    // Text rendering
    Error setTextColor(Color foreground, Color background);
    Error setTextSize(uint8_t size);
    Error drawText(Point pos, const char* text);
    Error drawTextCentered(Point center, const char* text);

    // Battery indicator
    Error drawBatteryIndicator(Point pos, int level, bool charging);

    // Display info
    Size getSize() const { return _size; }
    bool isInitialized() const { return _initialized; }

private:
    Display() = default;
    ~Display() = default;
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;

    bool _initialized = false;
    Size _size = {240, 135};  // Default size (landscape for M5StickC PLUS2)
    Color _textColor = Color::White();
    Color _textBgColor = Color::Black();
    uint8_t _textSize = 1;
    uint8_t _brightness = 100;
};

} // namespace Core
} // namespace NightStrike

