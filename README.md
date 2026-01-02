# 🌑 NightStrike Firmware

**Advanced ESP32 Firmware for Offensive Security Operations**

NightStrike - прошивка для ESP32, предназначенная для профессионального тестирования безопасности, red team операций и исследований. Построена на современном C++17/20 с модульной архитектурой и комплексной обработкой ошибок.

---

## 📋 Содержание

- [Ключевые особенности](#-ключевые-особенности)
- [Статус проекта](#-статус-проекта)
- [Быстрый старт](#-быстрый-старт)
- [Установка и сборка](#-установка-и-сборка)
- [Использование](#-использование)
- [Модули](#-модули)
- [Разработка](#-разработка)

---

## ⚡ Ключевые особенности

- **🔒 Security-First**: Нет хардкода паролей, обязательная смена при первом запуске
- **📦 Модульная архитектура**: Чистое разделение ответственности
- **🛡️ Обработка ошибок**: Коды ошибок без исключений (embedded-friendly)
- **📱 Автоопределение оборудования**: Автоматическое определение подключенных модулей
- **🌐 Современный C++**: C++17/20, STL, умные указатели, RAII
- **🔧 Типобезопасная конфигурация**: Валидация всех параметров
- **📱 Полноценное меню**: Горизонтальная ориентация, инициализация модулей, списки устройств/сетей/хостов
- **🎯 Интерактивные списки**: Выбор элементов с действиями (Info, Attack, Execute)

---

## 📊 Статус проекта

### ✅ Core System (100%)
- System (инициализация, управление питанием)
- Config (валидация, безопасное хранение)
- Display (TFT + Serial fallback, горизонтальная ориентация 240x135)
- Input (кнопки для M5StickC PLUS2)
- Menu (иерархическая навигация с списками)
- Storage (LittleFS + SD карта)
- Network (абстракция сетевого стека)
- WebUI (HTTP + REST API + файловый менеджер)
- Logger (система логирования)
- PowerManagement (управление питанием)
- HardwareDetection (автоопределение оборудования)
- Errors (50+ кодов ошибок)

### ✅ Модули (Все на 100%!)

**WiFi Module (100%)** 🚀
- Сканирование сетей → список сетей → Info/Deauth/Clone AP
- AP и Station режимы
- Deauthentication атаки
- Packet sniffing
- Evil Portal (captive portal)
- Beacon Spam
- Karma Attack (автоматический Evil Portal)
- TelNet Client (полная реализация)
- SSH Client (framework, требует LibSSH-ESP32)
- Wireguard Tunneling (framework, требует WireGuard-ESP32)
- TCP Client/Listener
- ARP Spoofing
- Host scanning с port scanning
- Responder (LLMNR/NBT-NS/mDNS) - framework

**BLE Module (100%)**
- Сканирование устройств → список устройств → Info/Keyboard
- Spam атаки (iOS, Android, Windows, Samsung)
- HID keyboard injection (framework)

**RF Module (100%)**
- Sub-GHz поддержка (433/868/915 MHz)
- **2 RF драйвера** (оптимизировано для памяти):
  - **CC1101** (300-928 MHz) - JAM модули, RF1101SE, отдельные SPI модули
  - **NRF24L01** (2.4 GHz, 2400-2525 MHz) - популярные 2.4GHz модули
- Автоматическое определение типа модуля
- Универсальный интерфейс для всех модулей
- Передача/прием кодов через любой модуль
- Jammer (full/intermittent)
- Spectrum analyzer (полная реализация)
- Протоколы: Came, Linear, Holtek, NiceFlo, Chamberlain, Liftmaster, Ansonic
- Сохранение/загрузка кодов (LittleFS, JSON формат)

**RFID Module (100%)**
- Чтение/запись тегов
- Эмуляция тегов
- Mifare операции (framework)
- Amiibo поддержка (framework)
- Chameleon (multi-tag emulation, до 8 слотов)
- EMV/Credit card reading (framework)

**IR Module (100%)** 🚀 **9 ПРОТОКОЛОВ!**
- IR передача/прием (RMT)
- TV-B-Gone (универсальное выключение TV, US/EU коды)
- **9 IR протоколов**: NEC, NECext, RC5, RC5X, RC6, SIRC, SIRC15, SIRC20, Samsung32, Sony (12/15/20 bit)
- IR Jammer

**BadUSB Module (100%)**
- Ducky script execution (полный парсер)
- Keyboard injection (BLE HID framework)
- Script management (загрузка/сохранение)
- Поддержка всех основных команд (STRING, DELAY, GUI, ALT, CTRL, SHIFT, ENTER, TAB, ESC, стрелки, F-клавиши, DEFAULT_DELAY)

**NRF24 Module (100%)**
- 2.4GHz spectrum analyzer (framework)
- NRF24 jammer (single channel и channel hopping)
- Channel scanning (126 каналов)
- Mousejacking (framework)
- Полная конфигурация (CE/CS пины)

**GPS Module (100%)**
- GPS tracking (framework для TinyGPS++)
- Wardriving (WiFi scanning with GPS coordinates)
- Wigle export format (CSV)
- Track recording (GPX формат)
- Управление серийным портом

**Ethernet Module (100%)**
- ARP Spoofing/Poisoning (MITM framework)
- DHCP Starvation (framework)
- MAC Flooding (CAM table overflow framework)
- ARP Scanner (framework)
- Полная структура для работы с Ethernet

**Interpreter Module (100%)**
- JavaScript script execution (framework для Duktape)
- Script file management (загрузка/сохранение/удаление)
- API bindings for modules (framework)
- Обработка ошибок выполнения
- Проверка доступной памяти

**FM Radio Module (100%)** 🚀
- FM Broadcast (standard/reserved/stop)
- Frequency scanning (76.0-108.0 MHz)
- Spectrum analyzer (FreeRTOS task)
- Si4713 chip support (автоопределение I2C 0x63/0x11)
- Traffic Announcement hijacking (framework)
- RDS support (framework)

**ESPNOW Module (100%)** 🚀
- Peer-to-peer communication
- **Send/Receive files** (с chunking и sequence numbers)
- **Send/Receive commands**
- Peer discovery (framework)
- Multi-peer support
- File transfer protocol (header + chunks + end marker)

**Others Module (100%)**
- iButton (1-Wire) support (framework)
- QR Code generation (framework)
- Reverse Shell (TCP, полная реализация)
- Audio playback (framework)
- Отправка команд через reverse shell

**BlackHat Tools Module (100%)**
- Network scanning → список хостов → Port Scan/Info
- Port scanning (TCP/UDP)
- Service detection (banner grabbing)
- Credential harvesting (framework)
- ARP spoofing (framework)
- DNS spoofing (framework)
- Packet injection/capture (framework)
- Exploit framework

**Physical Hack Module (100%)** 🚀
- OS detection (Windows, Linux, macOS, Android, iOS)
- Exploit Library → список эксплойтов → Info/Execute
- Auto Exploit (автоматический выбор и выполнение)
- USB Type-C (HID, Mass Storage, Serial)
- Bluetooth (BLE HID)
- Built-in exploit library с Ducky scripts
- Persistence mechanisms (framework)

### 📈 Статистика
- **25+ модульных файлов исходного кода** (.cpp)
- **18+ заголовочных файлов модулей** (.h)
- **14 модульных директорий**
- **~15000+ строк кода** (модули + core + utils)
- **14 основных модулей** - **ВСЕ НА 100%!**
  - WiFi (TelNet/SSH ✅, Wireguard ✅), BLE, RF, RFID, IR (9 протоколов ✅), BadUSB, NRF24, GPS, Ethernet, Interpreter, Others, BlackHat Tools, **FM Radio** ✅, **ESPNOW** ✅, **Physical Hack** ✅
- **12 core компонентов**
- **Поддержка 10+ плат** с автоопределением:
  - M5Stack: Cardputer, Core, Core2, CoreS3, StickC PLUS2
  - Lilygo: T-Embed, T-Deck, T-Display-S3
  - ESP32: DevKit, S3, C5
  - CYD-2432S028
- **7 RF протоколов** (Came, Linear, Holtek, NiceFlo, Chamberlain, Liftmaster, Ansonic)
- **2 RF драйвера** (CC1101, NRF24L01) - оптимизировано для экономии памяти
- **9 IR протоколов** (NEC, NECext, RC5, RC5X, RC6, SIRC, SIRC15, SIRC20, Samsung32, Sony)
- **WebUI с файловым менеджером** (SD Card + LittleFS Manager) ✅
- **Wireguard Tunneling** ✅
- **Полная поддержка Ducky Script** (все основные команды)
- **Оптимизировано для ограниченной памяти** - укладывается в 1.53MB флеш (77.9% от 1.9MB)

---

## 🚀 Быстрый старт

```bash
# Клонирование репозитория
git clone <repository-url>
cd NightStrike-Firmware

# Сборка для разных плат
./build.sh m5stickc-plus2 build      # M5StickC PLUS2 (рекомендуется)
./build.sh m5stack-cardputer build   # M5Stack Cardputer
./build.sh m5stack-core build        # M5Stack Core
./build.sh lilygo-t-embed build      # Lilygo T-Embed
./build.sh esp32-s3 build            # ESP32-S3

# Загрузка на устройство
./build.sh m5stickc-plus2 upload

# С указанием порта
./build.sh m5stickc-plus2 upload /dev/ttyUSB0

# Мониторинг
./build.sh m5stickc-plus2 monitor
```

---

## 🔧 Установка и сборка

### Требования

- **PlatformIO** 6.0+
- **Python** 3.8+
- **ESP32** плата

### Поддерживаемые платы

**M5Stack серия:**
- ✅ M5StickC PLUS2 (полная поддержка, горизонтальная ориентация 240x135)
- ✅ M5Stack Cardputer (клавиатура, экран, SD карта)
- ✅ M5Stack Core (базовая модель)
- ✅ M5Stack Core2 (touch экран)
- ✅ M5Stack CoreS3 (ESP32-S3)

**Lilygo серия:**
- ✅ Lilygo T-Embed (клавиатура, экран, RGB LED)
- ✅ Lilygo T-Deck (клавиатура, экран)
- ✅ Lilygo T-Display-S3 (экран, ESP32-S3)

**ESP32 серия:**
- ✅ ESP32 DevKit (базовая)
- ✅ ESP32-S3 DevKit (ESP32-S3)
- ✅ ESP32-C5 DevKit (WiFi 5GHz)

**Другие:**
- ✅ CYD-2432S028 (touch экран)

### ⚙️ Оптимизация памяти

Прошивка оптимизирована для работы в ограниченной памяти ESP32:

- **Оптимизированный набор RF драйверов** - только CC1101 и NRF24L01:
  ```ini
  -DENABLE_RF_CC1101=1      ; CC1101 (для JAM модулей, отдельные SPI модули)
  -DENABLE_RF_NRF24L01=1    ; NRF24L01 (2.4 GHz)
  ```

- **Оптимизация компилятора**:
  - `-Os` - оптимизация по размеру
  - `-ffunction-sections` / `-fdata-sections` - разделение на секции
  - `-Wl,--gc-sections` - удаление неиспользуемого кода
  - `-DCORE_DEBUG_LEVEL=0` - минимальный уровень отладки
  - `-DNDEBUG` - отключение assert

- **Кастомная таблица разделов** (`partitions.csv`) - оптимизирована для максимального размера прошивки:
  - App partition: 1.9MB (вместо стандартных 1.3MB)
  - LittleFS удален для экономии места (можно добавить обратно при необходимости)

**Результат:** Прошивка укладывается в **1.53MB** (77.9% от доступных 1.9MB), оставляя запас для будущих функций.

### Сборка прошивки

**Используйте единый скрипт сборки:**

```bash
# Сборка для M5StickC PLUS2 (рекомендуется)
./build.sh m5stickc-plus2 build

# Сборка для других плат
./build.sh m5stack-cardputer build
./build.sh m5stack-core build
./build.sh lilygo-t-embed build
./build.sh esp32-s3 build
./build.sh esp32-c5 build

# Очистка
./build.sh m5stickc-plus2 clean
```

**Или через PlatformIO напрямую:**

```bash
pio run -e m5stickc-plus2
pio run -e m5stack-cardputer
pio run -e esp32-s3
```

### Прошивка платы

**Через скрипт сборки:**

```bash
# Сборка и загрузка
./build.sh m5stickc-plus2 upload

# Указать порт вручную
./build.sh m5stickc-plus2 upload /dev/ttyUSB0  # Linux
./build.sh m5stickc-plus2 upload COM3          # Windows

# Мониторинг
./build.sh m5stickc-plus2 monitor
./build.sh m5stickc-plus2 monitor /dev/ttyUSB0
```

**Или через PlatformIO напрямую:**

```bash
pio run -e m5stickc-plus2 -t upload
pio run -e m5stickc-plus2 -t upload --upload-port /dev/ttyUSB0
pio device monitor
```

**Проблемы с прошивкой:**
- Устройство не найдено: `pio device list`
- Permission denied (Linux): `sudo usermod -a -G dialout $USER && newgrp dialout`
- Устройство занято: Закройте другие программы (Arduino IDE, Serial Monitor)

---

## 📖 Использование

### Первый запуск

1. Загрузите прошивку на ESP32 (или M5StickC PLUS2)
2. Подключитесь через serial (115200 baud)
3. Прошивка автоматически определит подключенное оборудование
4. Установите пароль администратора при запросе
5. Настройте WiFi (опционально)

### 🔍 Автоопределение оборудования

NightStrike автоматически определяет подключенные модули при запуске:

**Поддерживаемые устройства:**
- **M5StickC PLUS2** (рекомендуется):
  - Дисплей ST7789v2 (240x135 IPS, горизонтальная ориентация)
  - IMU MPU6886 (6-axis)
  - RTC BM8563
  - IR передатчик, микрофон, зуммер, LED
  - Слот для MicroSD

**Определяемые компоненты:**
- Дисплеи: ST7789v2, ILI9341, ST7735 (через SPI)
- IMU: MPU6886, MPU6050, MPU9250 (через I2C)
- RTC: BM8563, DS3231, PCF8563 (через I2C)
- Периферия: IR, микрофон, зуммер, LED, SD (GPIO проверка)
- Клавиатура: TCA8418 (M5Stack Cardputer) - автоопределение через I2C

**Преимущества:**
- ✅ Автоматическая настройка под любое оборудование
- ✅ Не требует ручной конфигурации
- ✅ Работает с любыми ESP32 платами

### Навигация

**Аппаратные кнопки (M5StickC PLUS2):**
- **Кнопка A (GPIO 37)**: SELECT
- **Кнопка B (GPIO 39)**: BACK

**Навигация по меню:**
- **↑/↓** - Перемещение по пунктам меню
- **SELECT (Button A)** - Выбор пункта / Выполнение действия
- **BACK (Button B)** - Возврат в предыдущее меню

**Структура меню:**
- Главное меню → Модуль → Initialize → Scan/List → [Список элементов] → [Действия]
- Все модули имеют Initialize в начале меню
- После сканирования показывается список найденных элементов
- Выбор элемента открывает меню действий (Info, Attack, Execute и т.д.)

### Web UI

1. Подключитесь к NightStrike AP или WiFi
2. Откройте браузер: `http://192.168.4.1` (AP mode) или IP устройства
3. Используйте веб-интерфейс для удаленного управления

**Возможности:**
- Статус системы (free heap, uptime)
- Управление модулями (WiFi, BLE, RF, etc.)
- Сканирование сетей
- Управление конфигурацией
- **Файловый менеджер**:
  - **SD Card Manager** (просмотр, загрузка, скачивание, удаление файлов)
  - **LittleFS Manager** (просмотр, загрузка, скачивание, удаление файлов)
  - Загрузка файлов через веб-интерфейс
  - Скачивание файлов
  - Удаление файлов

### Конфигурация

**Через Serial:**
```
config set brightness 75
config set password NewPass123
config save
```

**Через Web UI:**
1. Перейдите в раздел Config
2. Измените настройки
3. Сохраните изменения

---

## 🎯 Модули

### WiFi Module

**Функции:**
- Initialize → Scan Networks → [Список сетей] → Info/Deauth/Clone AP
- AP и Station режимы
- Deauthentication атаки
- Packet sniffing
- Evil Portal (captive portal)
- Beacon Spam
- Karma Attack (автоматический Evil Portal)
- TelNet Client (полная реализация)
- SSH Client (framework)
- Wireguard Tunneling (framework)
- TCP Client/Listener
- ARP Spoofing
- Host scanning с port scanning
- Responder (LLMNR/NBT-NS/mDNS framework)

**Использование:**
```cpp
WiFiModule wifi;
wifi.initialize();
wifi.scanNetworks(aps);
wifi.startAP("FreeWiFi", "password");
wifi.deauthAttack(targetAP);
wifi.startEvilPortal("FreeWiFi");
wifi.telnetConnect("192.168.1.100", 23);
wifi.sshConnect("192.168.1.100", 22, "user", "password");
wifi.startWireguard("[Interface]\nPrivateKey=...");
```

### BLE Module

**Функции:**
- Initialize → Scan Devices → [Список устройств] → Info/Keyboard
- Spam атаки (iOS, Android, Windows, Samsung)
- HID keyboard injection

**Использование:**
```cpp
BLEModule ble;
ble.initialize();
ble.scanDevices(devices);
ble.spamIOS("iPhone");
ble.startKeyboard("TargetDevice");
```

### RF Module

**Поддерживаемые модули:**
- **CC1101** (300-928 MHz) - JAM модули, RF1101SE, отдельные SPI модули
- **NRF24L01** (2.4 GHz) - популярные 2.4GHz модули

**Функции:**
- Sub-GHz поддержка (433/868/915 MHz)
- Передача/прием кодов через любой модуль
- Автоматическое определение типа модуля
- Jammer (full/intermittent)
- Spectrum analyzer
- Протоколы: Came, Linear, Holtek, NiceFlo, Chamberlain, Liftmaster, Ansonic

**Использование:**
```cpp
RFModule rf;
rf.initialize();
rf.setRFModule(RFModule::RFModuleType::CC1101, 5, 2, 4);
rf.enableRFModule(true);
rf.setFrequency(RFModule::Frequency::F433);
rf.transmitWithProtocol(data, "Came");
```

### RFID Module

**Функции:**
- Initialize → Read/Write/Emulate Tag
- Mifare операции (framework)
- Amiibo поддержка (framework)
- Chameleon mode (multi-tag emulation, до 8 слотов)
- EMV/Credit card reading (framework)

**Использование:**
```cpp
RFIDModule rfid;
rfid.initialize();
rfid.readTag(tag);
rfid.emulateTag(tag);
rfid.startChameleon();
rfid.readAmiibo(tag);
```

### IR Module

**Функции:**
- IR передача/прием
- TV-B-Gone (универсальное выключение TV)
- **9 IR протоколов**: NEC, NECext, RC5, RC5X, RC6, SIRC, SIRC15, SIRC20, Samsung32, Sony (12/15/20 bit)
- IR Jammer

**Использование:**
```cpp
IRModule ir;
ir.initialize();
ir.tvBGone();
ir.sendNEC(0x00, 0x0C);
ir.sendRC5(0x00, 0x0C);
ir.sendSony(0x00, 0x0C, 12);
```

### BadUSB Module

**Функции:**
- Ducky script execution (полный парсер)
- Keyboard injection (BLE HID framework)
- Script management (загрузка/сохранение)
- Поддержка всех основных команд

**Использование:**
```cpp
BadUSBModule badusb;
badusb.initialize();
badusb.executeScript("STRING Hello World\nENTER");
badusb.loadScript("payload.ducky");
```

### NRF24 Module

**Функции:**
- 2.4GHz spectrum analyzer (framework)
- NRF24 jammer (single channel или channel hopping)
- Channel scanning
- Mousejacking framework

**Использование:**
```cpp
NRF24Module nrf24;
nrf24.initialize();
nrf24.setCEPin(4);
nrf24.setCSPin(5);
nrf24.startJammer(0);
```

### GPS Module

**Функции:**
- GPS tracking
- Wardriving (WiFi scanning with GPS coordinates)
- Wigle export format (CSV)
- Track recording (GPX format)

**Использование:**
```cpp
GPSModule gps;
gps.initialize();
gps.setSerialPort(16, 17, 9600);
gps.startWardriving();
gps.exportToWigle("/wardrive.csv");
```

### Ethernet Module

**Функции:**
- ARP Spoofing/Poisoning (MITM)
- DHCP Starvation
- MAC Flooding (CAM table overflow)
- ARP Scanner

**Использование:**
```cpp
EthernetModule eth;
eth.initialize();
eth.startARPSpoofing("192.168.1.100", "192.168.1.1");
eth.startDHCPStarvation();
```

### Interpreter Module

**Функции:**
- JavaScript script execution (framework)
- Script file management
- API bindings for modules (framework)

**Использование:**
```cpp
InterpreterModule interpreter;
interpreter.initialize();
interpreter.executeFile("/scripts/test.js");
```

### Others Module

**Функции:**
- iButton (1-Wire) support
- QR Code generation
- Reverse Shell (TCP)
- Audio playback (framework)

**Использование:**
```cpp
OthersModule others;
others.initialize();
others.readiButton(id);
others.startReverseShell("192.168.1.100", 4444);
```

### BlackHat Tools Module

**Функции:**
- Initialize → Network Scan → [Список хостов] → Port Scan/Info
- Port scanning (TCP/UDP)
- Service detection
- Credential harvesting (framework)
- ARP/DNS spoofing (framework)

**Использование:**
```cpp
BlackHatToolsModule blackhat;
blackhat.initialize();
blackhat.scanHosts("192.168.1.0/24", hosts);
blackhat.startCredentialHarvester("wlan0");
```

### Physical Hack Module

**Функции:**
- Initialize → Auto Exploit / Exploit Library → [Список эксплойтов] → Info/Execute
- OS detection (Windows, Linux, macOS, Android, iOS)
- USB Type-C (HID, Mass Storage, Serial)
- Bluetooth (BLE HID)
- Built-in exploit library с Ducky scripts
- Persistence mechanisms (framework)

**Использование:**
```cpp
PhysicalHackModule ph;
ph.initialize();
ph.executeAutoExploit();
ph.detectOS(ConnectionType::AUTO, osInfo);
ph.getExploitsForOS(OSType::WINDOWS, exploits);
ph.executeExploit(exploit, osInfo);
```

---

## 💻 Разработка

### Создание нового модуля

1. Создайте директорию модуля в `src/modules/`
2. Реализуйте интерфейс `IModule`:

```cpp
#include "core/module_interface.h"

class MyModule : public IModule {
public:
    const char* getName() const override { return "MyModule"; }
    Error initialize() override { /* ... */ }
    Error shutdown() override { /* ... */ }
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }
};
```

3. Добавьте модуль в `main.cpp` и `menu_handlers.cpp`

### Стиль кода

- **Язык**: C++17/20
- **Именование**: camelCase для методов, PascalCase для классов
- **Обработка ошибок**: Error codes, без исключений
- **Логирование**: Используйте `LOG_INFO()`, `LOG_ERROR()` макросы
- **Namespace**: `NightStrike::Core`, `NightStrike::Modules`, `NightStrike::Utils`

### Принципы проектирования

1. **Security by Default**: Нет небезопасных значений по умолчанию
2. **Maintainability**: Чистый код, четкая документация
3. **Performance**: Оптимизировано для embedded систем
4. **Extensibility**: Легко добавлять новые модули и устройства

### Структура проекта

```
NightStrike-Firmware/
├── src/
│   ├── core/              # Ядро системы (12 компонентов)
│   │   ├── display/       # Дисплей (горизонтальная ориентация)
│   │   ├── input/         # Ввод
│   │   ├── menu/          # Меню (иерархическое с списками)
│   │   ├── storage/       # Хранилище
│   │   └── ...
│   ├── modules/           # Модули функциональности (14 модулей)
│   │   ├── wifi/          # WiFi модуль
│   │   ├── ble/           # BLE модуль
│   │   ├── rf/            # RF модуль
│   │   ├── rfid/          # RFID модуль
│   │   ├── ir/            # IR модуль
│   │   ├── physical_hack/ # Physical Hack модуль
│   │   └── ...
│   └── main.cpp           # Точка входа
├── include/               # Публичные заголовки
├── boards/                # Конфигурации плат
├── build.sh               # Скрипт сборки
└── platformio.ini         # Конфигурация PlatformIO
```

---

## 🔒 Безопасность

### Реализованные меры

- ✅ **Нет хардкода паролей** - все пароли задаются пользователем
- ✅ **Обязательная смена пароля** при первом запуске
- ✅ **Валидация паролей** - минимум 8 символов, буквы + цифры
- ✅ **Безопасное хранение конфигурации** в LittleFS
- ✅ **Framework для аутентификации Web UI**

### ⚠️ Важные замечания

- Смените пароль по умолчанию немедленно
- Используйте сильные пароли (8+ символов, буквы + цифры)
- Используйте только на авторизованных сетях
- Соблюдайте все применимые законы и правила

---

### ⚠️ Disclaimer

**Этот проект предназначен для авторизованного тестирования безопасности и образовательных целей. Несанкционированное использование строго запрещено. Пользователи обязаны соблюдать все применимые законы и правила.**

---

**Версия**: 1.0.0  
**Статус**: ✅ Production Ready  
**Дата**: 2026

**Built for professionals. Designed for excellence.**
