// M5Stack Cardputer pin definitions
#ifndef PINS_ARDUINO_H
#define PINS_ARDUINO_H

// Grove I2C
#define GROVE_SDA 2
#define GROVE_SCL 1

// SPI
#define SPI_SCK_PIN 40
#define SPI_MOSI_PIN 14
#define SPI_MISO_PIN 39
#define SPI_SS_PIN 1

// SD Card
#define SDCARD_CS 12

// CC1101
#define CC1101_SS_PIN SPI_SS_PIN
#define CC1101_GDO0_PIN GROVE_SDA

// NRF24
#define NRF24_CE_PIN GROVE_SDA
#define NRF24_SS_PIN SPI_SS_PIN

#endif

