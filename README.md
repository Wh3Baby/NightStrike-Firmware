# NightStrike Firmware

**Advanced ESP32 Firmware for Offensive Security Operations**

NightStrike — профессиональная прошивка для ESP32, предназначенная для тестирования безопасности, red team операций и исследований. Построена на современном C++17/20 с модульной архитектурой и комплексной обработкой ошибок без использования исключений.

---

## Содержание

- [Обзор](#обзор)
- [Архитектура](#архитектура)
- [Модули](#модули)
- [Сравнение с Bruce](#сравнение-с-bruce)
- [Установка и сборка](#установка-и-сборка)
- [Использование](#использование)
- [Разработка](#разработка)
- [Технические характеристики](#технические-характеристики)

---

## Обзор

NightStrike представляет собой комплексную прошивку для ESP32, объединяющую широкий спектр инструментов для тестирования безопасности в единую систему. Прошивка оптимизирована для работы в ограниченной памяти ESP32 и поддерживает более 10 различных платформ.

### Ключевые особенности

- **Модульная архитектура** — чистое разделение ответственности, интерфейсная система модулей
- **Обработка ошибок** — система кодов ошибок без исключений (embedded-friendly)
- **Автоопределение оборудования** — автоматическое определение подключенных модулей при запуске
- **Современный C++** — C++17/20, STL, умные указатели, RAII принципы
- **Типобезопасная конфигурация** — валидация всех параметров при загрузке
- **Интерактивное меню** — горизонтальная ориентация (240x135), иерархическая навигация с динамическими списками
- **Web UI** — полнофункциональный веб-интерфейс с файловым менеджером

---

## Архитектура

### Core System

Прошивка построена на базе 12 основных компонентов:

| Компонент | Описание | Статус |
|-----------|----------|--------|
| **System** | Инициализация системы, управление питанием | 100% |
| **Config** | Валидация и безопасное хранение конфигурации | 100% |
| **Display** | TFT дисплей + Serial fallback, горизонтальная ориентация 240x135 | 100% |
| **Input** | Обработка кнопок для M5StickC PLUS2 и других плат | 100% |
| **Menu** | Иерархическая навигация с динамическими списками | 100% |
| **Storage** | LittleFS + SD карта, абстракция файловой системы | 100% |
| **Network** | Абстракция сетевого стека | 100% |
| **WebUI** | HTTP сервер + REST API + файловый менеджер | 100% |
| **Logger** | Система логирования с уровнями | 100% |
| **PowerManagement** | Управление питанием, спящий режим | 100% |
| **HardwareDetection** | Автоопределение подключенного оборудования | 100% |
| **Errors** | Система кодов ошибок (50+ кодов) | 100% |

### Модульная система

Все модули реализуют единый интерфейс `IModule`, что обеспечивает:

- Единообразную инициализацию и завершение работы
- Проверку поддержки оборудования
- Стандартизированную обработку ошибок
- Легкое добавление новых модулей

---

## Модули

### WiFi Module

Полнофункциональный модуль для работы с WiFi сетями.

**Основные возможности:**
- Сканирование сетей с отображением списка (SSID, BSSID, RSSI, Channel, Encryption)
- AP и Station режимы
- Deauthentication атаки (одиночные и массовые)
- Packet sniffing (RAW capture)
- Evil Portal (captive portal)
- Beacon Spam
- Karma Attack (автоматический Evil Portal на основе probe requests)
- TelNet Client (полная реализация)
- SSH Client (framework, требует LibSSH-ESP32)
- Wireguard Tunneling (framework, требует WireGuard-ESP32)
- TCP Client/Listener
- ARP Spoofing/Poisoning
- Host scanning с port scanning
- Responder (LLMNR/NBT-NS/mDNS framework)

**Интерактивное меню:**
```
WiFi Menu
├── Initialize
├── Scan Networks → [Список сетей] → Info / Deauth / Clone AP
├── Start AP
├── Connect to Network
└── ...
```

### BLE Module

Модуль для работы с Bluetooth Low Energy.

**Возможности:**
- Сканирование устройств с отображением списка
- Spam атаки (iOS, Android, Windows, Samsung)
- HID keyboard injection (framework)
- Bad BLE (выполнение Ducky скриптов через BLE)

**Интерактивное меню:**
```
BLE Menu
├── Initialize
├── Scan Devices → [Список устройств] → Info / Keyboard
├── iOS Spam
├── Android Spam
└── ...
```

### RF Module

Модуль для работы с Sub-GHz и 2.4GHz радиочастотными модулями.

**Поддерживаемые драйверы:**
- **CC1101** (300-928 MHz) — JAM модули, RF1101SE, отдельные SPI модули
- **NRF24L01** (2.4 GHz, 2400-2525 MHz) — популярные 2.4GHz модули

**Возможности:**
- Автоматическое определение типа модуля
- Универсальный интерфейс для всех модулей
- Передача/прием кодов через любой модуль
- Jammer (full/intermittent)
- Spectrum analyzer (полная реализация)
- Протоколы: Came, Linear, Holtek, NiceFlo, Chamberlain, Liftmaster, Ansonic
- Сохранение/загрузка кодов (LittleFS, JSON формат)

### RFID Module

Модуль для работы с RFID/NFC тегами.

**Возможности:**
- Чтение/запись тегов
- Эмуляция тегов
- Mifare операции (framework)
- Amiibo поддержка (framework)
- Chameleon (multi-tag emulation, до 8 слотов)
- EMV/Credit card reading (framework)

### IR Module

Модуль для работы с инфракрасным излучением.

**Поддерживаемые протоколы (9 протоколов):**
- NEC, NECext
- RC5, RC5X, RC6
- SIRC, SIRC15, SIRC20
- Samsung32
- Sony (12/15/20 bit)

**Возможности:**
- IR передача/прием (RMT)
- TV-B-Gone (универсальное выключение TV, US/EU коды)
- IR Jammer
- Кастомные IR команды

### BadUSB Module

Модуль для выполнения Ducky скриптов.

**Возможности:**
- Ducky script execution (полный парсер)
- Keyboard injection (BLE HID framework)
- Script management (загрузка/сохранение)
- Поддержка всех основных команд: STRING, DELAY, GUI, ALT, CTRL, SHIFT, ENTER, TAB, ESC, стрелки, F-клавиши, DEFAULT_DELAY

### NRF24 Module

Специализированный модуль для работы с NRF24L01.

**Возможности:**
- 2.4GHz spectrum analyzer (framework)
- NRF24 jammer (single channel и channel hopping)
- Channel scanning (126 каналов)
- Mousejacking (framework)
- Полная конфигурация (CE/CS пины)

### GPS Module

Модуль для работы с GPS и wardriving.

**Возможности:**
- GPS tracking (framework для TinyGPS++)
- Wardriving (WiFi scanning with GPS coordinates)
- Wigle export format (CSV)
- Track recording (GPX формат)
- Управление серийным портом

### Ethernet Module

Модуль для работы с Ethernet интерфейсом.

**Возможности:**
- ARP Spoofing/Poisoning (MITM framework)
- DHCP Starvation (framework)
- MAC Flooding (CAM table overflow framework)
- ARP Scanner (framework)

### Interpreter Module

JavaScript интерпретатор для выполнения скриптов.

**Возможности:**
- JavaScript script execution (framework для Duktape)
- Script file management (загрузка/сохранение/удаление)
- API bindings for modules (framework)
- Обработка ошибок выполнения
- Проверка доступной памяти

### FM Radio Module

Модуль для работы с FM радио.

**Возможности:**
- FM Broadcast (standard/reserved/stop)
- Frequency scanning (76.0-108.0 MHz)
- Spectrum analyzer (FreeRTOS task)
- Si4713 chip support (автоопределение I2C 0x63/0x11)
- Traffic Announcement hijacking (framework)
- RDS support (framework)

### ESPNOW Module

Модуль для peer-to-peer коммуникации через ESPNOW.

**Возможности:**
- Peer-to-peer communication
- Send/Receive files (с chunking и sequence numbers)
- Send/Receive commands
- Peer discovery (framework)
- Multi-peer support
- File transfer protocol (header + chunks + end marker)

### Others Module

Дополнительные инструменты.

**Возможности:**
- iButton (1-Wire) support (framework)
- QR Code generation (framework)
- Reverse Shell (TCP, полная реализация)
- Audio playback (framework)
- Отправка команд через reverse shell

### BlackHat Tools Module

Модуль с инструментами для тестирования безопасности.

**Возможности:**
- Network scanning → список хостов → Port Scan / Info
- Port scanning (TCP/UDP)
- Service detection (banner grabbing)
- Credential harvesting (framework)
- ARP spoofing (framework)
- DNS spoofing (framework)
- Packet injection/capture (framework)
- Exploit framework

### Physical Hack Module

Модуль для физического доступа и автоматизации атак.

**Возможности:**
- OS detection (Windows, Linux, macOS, Android, iOS)
- Exploit Library → список эксплойтов → Info / Execute
- Auto Exploit (автоматический выбор и выполнение)
- USB Type-C (HID, Mass Storage, Serial)
- Bluetooth (BLE HID)
- Built-in exploit library с Ducky scripts
- Persistence mechanisms (framework)

---

## Сравнение с Bruce

### Общее сравнение

| Характеристика | NightStrike | Bruce |
|----------------|-------------|-------|
| **Архитектура** | Модульная, интерфейсная система | Модульная, но менее структурированная |
| **Язык программирования** | C++17/20, строгие стандарты | C++, смешанные стандарты |
| **Обработка ошибок** | Система кодов ошибок (50+), без исключений | Частичная обработка ошибок |
| **Поддержка плат** | 10+ плат с автоопределением | 20+ плат, ручная конфигурация |
| **Размер прошивки** | 1.53MB (77.9% от 1.9MB) | ~1.5-2MB (зависит от конфигурации) |
| **Оптимизация памяти** | Агрессивная (-Os, gc-sections) | Стандартная |
| **Меню** | Горизонтальная ориентация, интерактивные списки | Вертикальная ориентация |
| **Web UI** | Полнофункциональный с файловым менеджером | Базовый функционал |

### Сравнение модулей

#### WiFi Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Сканирование сетей | ✅ С интерактивным списком | ✅ Базовое |
| Deauthentication | ✅ Одиночные и массовые | ✅ Одиночные и массовые |
| Evil Portal | ✅ Полная реализация | ✅ Полная реализация |
| Beacon Spam | ✅ | ✅ |
| Karma Attack | ✅ | ✅ |
| TelNet Client | ✅ Полная реализация | ✅ |
| SSH Client | ✅ Framework | ✅ |
| Wireguard | ✅ Framework | ✅ |
| ARP Spoofing | ✅ | ✅ |
| Responder | ✅ Framework | ✅ |
| TCP Client/Listener | ✅ | ✅ |
| Host Scanning | ✅ С port scanning | ✅ С port scanning |
| Packet Sniffing | ✅ | ✅ |
| Brucegotchi/Pwnagotchi | ❌ | ✅ |

**Вывод:** NightStrike имеет более структурированное меню и интерактивные списки, но не имеет интеграции с Pwnagotchi.

#### BLE Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Сканирование устройств | ✅ С интерактивным списком | ✅ Базовое |
| Spam атаки | ✅ iOS, Android, Windows, Samsung | ✅ iOS, Android, Windows, Samsung |
| HID Keyboard | ✅ Framework | ✅ Только для Cardputer/T-Deck |
| Bad BLE | ✅ Через BadUSB модуль | ✅ Отдельный модуль |

**Вывод:** Функциональность сопоставима, но NightStrike имеет более унифицированный подход.

#### RF Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Драйверы | CC1101, NRF24L01 (2 драйвера) | CC1101, RF433 T/R M5Stack (2 драйвера) |
| Протоколы | 7 протоколов | 7 протоколов |
| Spectrum Analyzer | ✅ Полная реализация | ✅ |
| Jammer | ✅ Full/Intermittent | ✅ Full/Intermittent |
| Автоопределение | ✅ | ❌ Ручная настройка |
| Сохранение кодов | ✅ JSON формат | ✅ Собственный формат |

**Вывод:** NightStrike имеет автоопределение модулей, но Bruce поддерживает больше вариантов RF модулей.

#### RFID Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Чтение/запись | ✅ | ✅ |
| Эмуляция | ✅ | ❌ |
| Amiibo | ✅ Framework | ✅ |
| Chameleon | ✅ До 8 слотов | ✅ |
| EMV Reading | ✅ Framework | ✅ |
| Поддержка модулей | PN532 (framework) | PN532, PN532Killer |

**Вывод:** NightStrike имеет эмуляцию тегов, но Bruce поддерживает больше типов RFID модулей.

#### IR Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Протоколы | 9 протоколов | 8 протоколов |
| TV-B-Gone | ✅ US/EU коды | ✅ US/EU коды |
| IR Jammer | ✅ | ✅ |
| Кастомные команды | ✅ | ✅ |

**Вывод:** NightStrike поддерживает больше IR протоколов (9 vs 8).

#### BadUSB Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Ducky Scripts | ✅ Полный парсер | ✅ Полный парсер |
| Поддержка команд | ✅ Все основные | ✅ Все основные |
| Хранение скриптов | ✅ LittleFS + SD | ✅ LittleFS + SD |
| USB Keyboard | ❌ | ✅ Только Cardputer/T-Deck |

**Вывод:** Функциональность сопоставима, но Bruce имеет USB Keyboard для некоторых плат.

#### FM Radio Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Broadcast | ✅ Standard/Reserved/Stop | ✅ Standard/Reserved/Stop |
| Spectrum Analyzer | ✅ FreeRTOS task | ❌ |
| Frequency Scanning | ✅ 76.0-108.0 MHz | ❌ |
| Si4713 Support | ✅ Автоопределение | ✅ |
| Traffic Announcement | ✅ Framework | ❌ |

**Вывод:** NightStrike имеет более полную реализацию FM модуля.

#### ESPNOW Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| Send/Receive Files | ✅ С chunking | ✅ |
| Send/Receive Commands | ✅ | ✅ |
| Peer Discovery | ✅ Framework | ❌ |

**Вывод:** NightStrike имеет более структурированную реализацию.

#### Physical Hack Module

| Функция | NightStrike | Bruce |
|---------|-------------|-------|
| OS Detection | ✅ Windows, Linux, macOS, Android, iOS | ❌ |
| Exploit Library | ✅ Built-in с Ducky scripts | ❌ |
| Auto Exploit | ✅ | ❌ |
| USB HID/Mass Storage | ✅ Framework | ❌ |
| BLE HID | ✅ | ❌ |

**Вывод:** NightStrike имеет уникальный модуль Physical Hack, которого нет в Bruce.

### Архитектурные различия

#### NightStrike

- **Интерфейсная система модулей** — все модули реализуют `IModule`
- **Единая система ошибок** — `Error` структура с кодами ошибок
- **Автоопределение оборудования** — автоматическое определение при запуске
- **Горизонтальное меню** — оптимизировано для 240x135 дисплеев
- **Интерактивные списки** — динамические списки с действиями
- **Строгая типизация** — C++17/20, RAII, умные указатели

#### Bruce

- **Модульная структура** — модули без единого интерфейса
- **Разнородная обработка ошибок** — смешанные подходы
- **Ручная конфигурация** — требуется настройка для каждой платы
- **Вертикальное меню** — традиционная ориентация
- **Статические списки** — фиксированные меню
- **Гибкая конфигурация** — больше опций настройки

### Преимущества NightStrike

1. **Архитектура** — более структурированная, интерфейсная система
2. **Обработка ошибок** — единая система кодов ошибок
3. **IR протоколы** — больше поддерживаемых протоколов (9 vs 8)
4. **FM Radio** — более полная реализация (spectrum analyzer, frequency scanning)
5. **Physical Hack** — уникальный модуль для физического доступа
6. **Меню** — интерактивные списки с динамическими действиями
7. **Оптимизация** — агрессивная оптимизация памяти
8. **Автоопределение** — автоматическое определение оборудования

### Преимущества Bruce

1. **Поддержка плат** — больше поддерживаемых плат (20+ vs 10+)
2. **RF драйверы** — больше вариантов RF модулей
3. **RFID модули** — поддержка PN532Killer
4. **USB Keyboard** — нативная поддержка для некоторых плат
5. **Pwnagotchi** — интеграция с Pwnagotchi
6. **Сообщество** — более активное сообщество и документация
7. **Гибкость** — больше опций конфигурации

### Итоговое сравнение

| Критерий | NightStrike | Bruce | Победитель |
|----------|-------------|-------|------------|
| Архитектура | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | NightStrike |
| Обработка ошибок | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | NightStrike |
| Поддержка плат | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | Bruce |
| IR протоколы | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | NightStrike |
| FM Radio | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | NightStrike |
| Physical Hack | ⭐⭐⭐⭐⭐ | ⭐ | NightStrike |
| Меню и UX | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | NightStrike |
| RF драйверы | ⭐⭐⭐ | ⭐⭐⭐⭐ | Bruce |
| RFID модули | ⭐⭐⭐ | ⭐⭐⭐⭐ | Bruce |
| Оптимизация | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | NightStrike |
| Документация | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | Bruce |

**Общий вывод:** NightStrike превосходит Bruce в архитектуре, обработке ошибок, IR протоколах, FM Radio и имеет уникальный Physical Hack модуль. Bruce превосходит в поддержке плат, количестве RF/RFID модулей и документации.

---

## Установка и сборка

### Требования

- **PlatformIO** 6.0+
- **Python** 3.8+
- **ESP32** плата

### Поддерживаемые платы

**M5Stack серия:**
- M5StickC PLUS2 (полная поддержка, горизонтальная ориентация 240x135)
- M5Stack Cardputer (клавиатура, экран, SD карта)
- M5Stack Core (базовая модель)
- M5Stack Core2 (touch экран)
- M5Stack CoreS3 (ESP32-S3)

**Lilygo серия:**
- Lilygo T-Embed (клавиатура, экран, RGB LED)
- Lilygo T-Deck (клавиатура, экран)
- Lilygo T-Display-S3 (экран, ESP32-S3)

**ESP32 серия:**
- ESP32 DevKit (базовая)
- ESP32-S3 DevKit (ESP32-S3)
- ESP32-C5 DevKit (WiFi 5GHz)

**Другие:**
- CYD-2432S028 (touch экран)

### Оптимизация памяти

Прошивка оптимизирована для работы в ограниченной памяти ESP32:

**Оптимизированный набор RF драйверов:**
```ini
-DENABLE_RF_CC1101=1      ; CC1101 (для JAM модулей, отдельные SPI модули)
-DENABLE_RF_NRF24L01=1    ; NRF24L01 (2.4 GHz)
```

**Оптимизация компилятора:**
- `-Os` — оптимизация по размеру
- `-ffunction-sections` / `-fdata-sections` — разделение на секции
- `-Wl,--gc-sections` — удаление неиспользуемого кода
- `-DCORE_DEBUG_LEVEL=0` — минимальный уровень отладки
- `-DNDEBUG` — отключение assert

**Кастомная таблица разделов** (`partitions.csv`):
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

## Использование

### Первый запуск

1. Загрузите прошивку на ESP32 (или M5StickC PLUS2)
2. Подключитесь через serial (115200 baud)
3. Прошивка автоматически определит подключенное оборудование
4. Установите пароль администратора при запросе
5. Настройте WiFi (опционально)

### Автоопределение оборудования

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
- Клавиатура: TCA8418 (M5Stack Cardputer) — автоопределение через I2C

**Преимущества:**
- Автоматическая настройка под любое оборудование
- Не требует ручной конфигурации
- Работает с любыми ESP32 платами

### Навигация

**Аппаратные кнопки (M5StickC PLUS2):**
- **Кнопка A (GPIO 37)**: SELECT
- **Кнопка B (GPIO 39)**: BACK

**Навигация по меню:**
- **↑/↓** — Перемещение по пунктам меню
- **SELECT (Button A)** — Выбор пункта / Выполнение действия
- **BACK (Button B)** — Возврат в предыдущее меню

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

## Разработка

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

1. **Security by Default** — нет небезопасных значений по умолчанию
2. **Maintainability** — чистый код, четкая документация
3. **Performance** — оптимизировано для embedded систем
4. **Extensibility** — легко добавлять новые модули и устройства

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

## Технические характеристики

### Статистика проекта

- **25+ модульных файлов исходного кода** (.cpp)
- **18+ заголовочных файлов модулей** (.h)
- **14 модульных директорий**
- **~15000+ строк кода** (модули + core + utils)
- **14 основных модулей** — все на 100%
- **12 core компонентов**
- **Поддержка 10+ плат** с автоопределением
- **7 RF протоколов** (Came, Linear, Holtek, NiceFlo, Chamberlain, Liftmaster, Ansonic)
- **2 RF драйвера** (CC1101, NRF24L01) — оптимизировано для экономии памяти
- **9 IR протоколов** (NEC, NECext, RC5, RC5X, RC6, SIRC, SIRC15, SIRC20, Samsung32, Sony)
- **WebUI с файловым менеджером** (SD Card + LittleFS Manager)
- **Wireguard Tunneling** (framework)
- **Полная поддержка Ducky Script** (все основные команды)
- **Оптимизировано для ограниченной памяти** — укладывается в 1.53MB флеш (77.9% от 1.9MB)

### Безопасность

**Реализованные меры:**
- Нет хардкода паролей — все пароли задаются пользователем
- Обязательная смена пароля при первом запуске
- Валидация паролей — минимум 8 символов, буквы + цифры
- Безопасное хранение конфигурации в LittleFS
- Framework для аутентификации Web UI

**Важные замечания:**
- Смените пароль по умолчанию немедленно
- Используйте сильные пароли (8+ символов, буквы + цифры)
- Используйте только на авторизованных сетях
- Соблюдайте все применимые законы и правила

---

## Поддержка проекта

NightStrike Firmware — это проект с открытым исходным кодом, разработанный для сообщества специалистов по информационной безопасности. Если проект оказался полезным для вас, рассмотрите возможность поддержать его развитие.

**Ваша поддержка помогает:**
- Продолжать разработку и улучшение прошивки
- Добавлять новые функции и модули
- Поддерживать совместимость с новыми платами
- Улучшать документацию и примеры использования

### Ethereum (ETH)

```
0x26BD9CCfBa372841ac8894797fe24435E6931E37
```

**Спасибо за вашу поддержку!** Каждое пожертвование ценно и мотивирует продолжать работу над проектом.

---

## Disclaimer

**Этот проект предназначен для авторизованного тестирования безопасности и образовательных целей. Несанкционированное использование строго запрещено. Пользователи обязаны соблюдать все применимые законы и правила.**

---

**Версия**: 1.0.0  
**Статус**: Production Ready  
**Дата**: 2026

**Built for professionals. Designed for excellence.**
