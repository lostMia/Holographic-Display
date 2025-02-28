

#include <Arduino.h>
#include <SPI.h>
#include "driver/spi_master.h"
#include "esp_log.h"
#include <cstring>

#define LED_COUNT 128
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
        .clock_speed_hz = 24 * 1000 * 1000,
        .spics_io_num = -1,                 // No CS pin
        .flags = SPI_DEVICE_HALFDUPLEX,      // Half-duplex for LED strips
        .queue_size = 1,                     // Only 1 transaction at a time
    };

   }

void updateLEDs() {
    if (!ledBuffer) return;

    // Start Frame (4 bytes of 0x00)
    memset(ledBuffer, 0x00, 4);

    // LED Data
    for (int i = 0; i < LED_COUNT; i++) {
        int offset = 4 + (i * 4);
        ledBuffer[offset] = 0xE0 | 4;  // Brightness (5 bits, max 31)
        ledBuffer[offset + 1] = 0x00;   // Blue
        ledBuffer[offset + 2] = 0xFF;   // Green
        ledBuffer[offset + 3] = 0x00;   // Red
    }

    // End Frame (4 bytes of 0xFF)
    memset(ledBuffer + 4 + (LED_COUNT * 4), 0xFF, 4);

    // Send data using DMA
    spi_transaction_t t = {
        .length = ((LED_COUNT * 4) + 8) * 8,  // Length in bits
        .user = NULL,
        .tx_buffer = ledBuffer               // Data to send
    };
    
    esp_err_t ret;

    unsigned long first, second, third, fourth, fifth;
    first = micros();

    // Blocking transfer.
    spi_device_polling_transmit(spi, &t);
    //
    // second = micros();
    //
    // // Non blocking transfer.
    // ret = spi_device_queue_trans(spi, &t, portMAX_DELAY);
    //
    // third = micros();
    //
    // if (ret == ESP_OK)
    //     Serial.println("No Error");
    // else
    //     Serial.println("Error!!");
    //
    // auto result = &t;
    //
    // do {
    //     ret = spi_device_get_trans_result(spi, &result, portMAX_DELAY);
    // } while (ret != ESP_OK);  // Keep checking until a transaction completes
    //
    // fourth = micros();
    //
    // spi_device_transmit(spi, &t);
    //
    // fifth = micros();
    //
    // printf("First: %d, Second: %d, Third: %d, Fourth: %d, Fifth: %d\n", first, second, third, fourth, fifth);
    // printf("Diff 1: %d, Diff 2: %d, Diff 3: %d, Diff 4: %d\n\n", second - first, third - second, fourth - third, fifth - fourth);

}

void setup() {
    Serial.begin(115200);

    setup_SPI();
    updateLEDs();  // Send initial LED data
}

void loop() {
    delay(100);
    
    Serial.println("looping..");
    ESP_LOGI("", "looping..");
    printf("looping\n");

    // Change colors dynamically
    for (int i = 0; i < LED_COUNT; i++) {
        int offset = 4 + (i * 4);
        ledBuffer[offset + 1] = 255;  // Random Blue
        ledBuffer[offset + 2] = 0;  // Random Green
        ledBuffer[offset + 3] = 0;  // Random Red
    }

    updateLEDs();  // Update the LED strip
}