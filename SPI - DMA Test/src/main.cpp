

#include <Arduino.h>
#include <SPI.h>
#include "driver/spi_master.h"
#include "esp_log.h"
#include <cstring>

#define LED_COUNT 127
#define SPI_HOST SPI3_HOST
#define DATA_PIN 7
#define CLOCK_PIN  4


spi_device_handle_t spi;

// DMA-capable buffer for LED data
uint8_t *ledBuffer = NULL; 

void setup_SPI() 
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = DATA_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CLOCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (LED_COUNT * 4) + 8,
    };


    spi_device_interface_config_t devcfg = {
        .mode = 0,                          // SPI mode 0 (CPOL=0, CPHA=0)
        .clock_speed_hz = 12 * 1000 * 1000, // 24 MHz
        .spics_io_num = -1,                 // No CS pin
        .flags = SPI_DEVICE_HALFDUPLEX,      // Half-duplex for LED strips
        .queue_size = 1,                     // Only 1 transaction at a time
    };

    // Initialize the SPI bus
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    // Attach the SPI device
    spi_bus_add_device(SPI_HOST, &devcfg, &spi);

    // Allocate DMA buffer
    ledBuffer = (uint8_t*)heap_caps_malloc((LED_COUNT * 4) + 8, MALLOC_CAP_DMA);
}

void updateLEDs() {
    if (!ledBuffer) return;

    // Start Frame (4 bytes of 0x00)
    memset(ledBuffer, 0x00, 4);

    // LED Data
    for (int i = 0; i < LED_COUNT; i++) {
        int offset = 4 + (i * 4);
        ledBuffer[offset] = 0xE0 | 31;  // Brightness (5 bits, max 31)
        ledBuffer[offset + 1] = 0x00;   // Blue
        ledBuffer[offset + 2] = 0xFF;   // Green
        ledBuffer[offset + 3] = 0x00;   // Red
    }

    // End Frame (at least LED_COUNT / 2 bytes of 0xFF)
    memset(ledBuffer + 4 + (LED_COUNT * 4), 0xFF, LED_COUNT / 2);

    // Send data using DMA
    spi_transaction_t t = {
        .length = ((LED_COUNT * 4) + 8) * 8,  // Length in bits
        .user = NULL,
        .tx_buffer = ledBuffer               // Data to send
    };
    spi_device_polling_transmit(spi, &t);  // Non-blocking DMA transfer
}

void setup() {
    Serial.begin(115200);
    setup_SPI();
    Serial.println("SPI DMA LED Strip Initialized");

    updateLEDs();  // Send initial LED data
}

void loop() {
    delay(1000);

    // Change colors dynamically
    for (int i = 0; i < LED_COUNT; i++) {
        int offset = 4 + (i * 4);
        ledBuffer[offset + 1] = random(256);  // Random Blue
        ledBuffer[offset + 2] = random(256);  // Random Green
        ledBuffer[offset + 3] = random(256);  // Random Red
    }

    updateLEDs();  // Update the LED strip
}