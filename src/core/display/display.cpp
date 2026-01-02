#include "core/display.h"
#include "core/hardware_detection.h"
#include <Arduino.h>

#ifdef HAS_SCREEN
#include <TFT_eSPI.h>
static TFT_eSPI tft;
#endif

namespace NightStrike {
namespace Core {

Display& Display::getInstance() {
    static Display instance;
    return instance;
}

Error Display::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    // Auto-detect display if not already detected
    auto& hwDetect = HardwareDetection::getInstance();
    if (!hwDetect.getInfo().boardName.empty()) {
        // Already detected
    } else {
        hwDetect.detectAll();
    }

    auto hwInfo = hwDetect.getInfo();

#ifdef HAS_SCREEN
    if (hwInfo.display == HardwareDetection::DisplayType::ST7789V2 ||
        hwInfo.display == HardwareDetection::DisplayType::UNKNOWN ||
        hwInfo.display != HardwareDetection::DisplayType::NONE) {
        // Try to initialize display
#ifdef M5STICKC_PLUS2
        // M5StickC PLUS2: ST7789v2, 240x135, rotation 3 (landscape)
        tft.init();
        tft.setRotation(3);  // Landscape mode (horizontal)
        tft.fillScreen(TFT_BLACK);
        _size.width = 240;
        _size.height = 135;
        Serial.printf("[Display] M5StickC PLUS2 TFT initialized (240x135 landscape)\n");
#else
        tft.init();
        tft.setRotation(1);
        _size.width = tft.width();
        _size.height = tft.height();
        tft.fillScreen(TFT_BLACK);
        Serial.printf("[Display] TFT initialized (%dx%d)\n", _size.width, _size.height);
#endif
    } else {
        Serial.println("[Display] No display detected");
        _size.width = 80;
        _size.height = 24;
    }
#else
    Serial.println("[Display] Running in serial-only mode");
    _size.width = 80;  // Terminal width
    _size.height = 24; // Terminal height
#endif

    _initialized = true;
    return Error(ErrorCode::SUCCESS);
}

Error Display::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    fillScreen(Color::Black());
    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

Error Display::setBrightness(uint8_t brightness) {
    if (brightness > 100) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

    _brightness = brightness;
#ifdef HAS_SCREEN
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: Control backlight via PWM (GPIO 10)
    // Map 0-100 to 0-255 for PWM
    uint8_t pwmValue = (brightness * 255) / 100;
    ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(10, 0);   // GPIO 10 for backlight
    ledcWrite(0, pwmValue);
#else
    // Generic TFT - brightness control depends on driver
    // Most TFT drivers don't support brightness control
#endif
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::setRotation(uint8_t rotation) {
#ifdef HAS_SCREEN
    tft.setRotation(rotation);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::fillScreen(Color color) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

#ifdef HAS_SCREEN
    tft.fillScreen(color.value);
#else
    Serial.println("[Display] Screen cleared");
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::clear() {
    return fillScreen(Color::Black());
}

Error Display::drawPixel(Point pos, Color color) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

#ifdef HAS_SCREEN
    tft.drawPixel(pos.x, pos.y, color.value);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::drawLine(Point start, Point end, Color color) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

#ifdef HAS_SCREEN
    tft.drawLine(start.x, start.y, end.x, end.y, color.value);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::drawRect(Point pos, Size size, Color color, bool filled) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

#ifdef HAS_SCREEN
    if (filled) {
        tft.fillRect(pos.x, pos.y, size.width, size.height, color.value);
    } else {
        tft.drawRect(pos.x, pos.y, size.width, size.height, color.value);
    }
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::drawCircle(Point center, uint16_t radius, Color color, bool filled) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

#ifdef HAS_SCREEN
    if (filled) {
        tft.fillCircle(center.x, center.y, radius, color.value);
    } else {
        tft.drawCircle(center.x, center.y, radius, color.value);
    }
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::setTextColor(Color foreground, Color background) {
    _textColor = foreground;
    _textBgColor = background;
#ifdef HAS_SCREEN
    tft.setTextColor(foreground.value, background.value);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::setTextSize(uint8_t size) {
    if (size == 0 || size > 7) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

    _textSize = size;
#ifdef HAS_SCREEN
    tft.setTextSize(size);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::drawText(Point pos, const char* text) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    if (!text) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

#ifdef HAS_SCREEN
    tft.setCursor(pos.x, pos.y);
    tft.print(text);
#else
    Serial.printf("[Display] Text: %s\n", text);
#endif
    return Error(ErrorCode::SUCCESS);
}

Error Display::drawTextCentered(Point center, const char* text) {
    if (!_initialized || !text) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

#ifdef HAS_SCREEN
    // Calculate text width and height manually
    // Approximate: width = strlen * char_width, height = font_height
    // For TFT_eSPI, typical char width is ~6 pixels per char at size 1
    uint16_t charWidth = 6 * _textSize;
    uint16_t charHeight = 8 * _textSize;
    uint16_t textWidth = strlen(text) * charWidth;
    uint16_t textHeight = charHeight;
    
    int16_t x = center.x - textWidth / 2;
    int16_t y = center.y - textHeight / 2;
    tft.setCursor(x, y);
    tft.print(text);
#else
    Serial.printf("[Display] Centered: %s\n", text);
#endif
    return Error(ErrorCode::SUCCESS);
}

} // namespace Core
} // namespace NightStrike

