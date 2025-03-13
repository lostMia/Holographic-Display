#include <Arduino.h>
#include "esp_log.h"
#include <cstring>

// #define SPI_DMA

#ifndef SPI_DMA
#include <FastLED.h>
#endif

#define NUM_LEDS 128
#define DATA_PIN 7
#define CLOCK_PIN  4
#define FREQUENCY 30

#ifndef SPI_DMA
CRGB leds[NUM_LEDS];
#endif

#ifdef SPI_DMA
#include <SPI.h>
#include "driver/spi_master.h"

#define SPI_HOST SPI3_HOST
#endif


#ifdef SPI_DMA
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
        .max_transfer_sz = (NUM_LEDS * 4) + 8,
    };


    spi_device_interface_config_t devcfg = {
        .mode = 0,                          // SPI mode 0 (CPOL=0, CPHA=0)
        .clock_speed_hz = FREQUENCY * 1000 * 1000,
        .spics_io_num = -1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
    };

    // Initialize the SPI bus
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Attach the SPI device
    spi_bus_add_device(SPI_HOST, &devcfg, &spi);

    // Allocate DMA buffer
    ledBuffer = (uint8_t*)heap_caps_malloc((NUM_LEDS * 4) + 8, MALLOC_CAP_DMA);

    // Start Frame (4 bytes of 0x00)
    memset(ledBuffer, 0x00, 4);

    // End Frame (4 bytes of 0xFF)
    memset(ledBuffer + 4 + (NUM_LEDS * 4), 0xFF, 4);
   }

void updateLEDs() {
    // Send data using DMA
    spi_transaction_t t = {
        .length = ((NUM_LEDS * 4) + 8) * 8,  // Length in bits
        .user = NULL,
        .tx_buffer = ledBuffer               // Data to send
    };
    
    esp_err_t ret;

    // unsigned long first, second, third, fourth, fifth;
    // first = micros();

    // Blocking transfer.
    spi_device_polling_transmit(spi, &t);
    //
    // second = micros();

    // // Non blocking transfer.
    // ret = spi_device_queue_trans(spi, &t, portMAX_DELAY);
    // //
    // third = micros();
    // 
    // auto trans = &t;
    //
    //  do {
    //     ret = spi_device_get_trans_result(spi, &trans, portMAX_DELAY);
    // } while (ret != ESP_OK);  // Keep checking until a transaction completes
    //
    // fourth = micros();
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
#endif

void setup() {
    Serial.begin(115200);

#ifdef SPI_DMA
    setup_SPI();
    updateLEDs();  // Send initial LED data
#else
    FastLED.addLeds<SK9822, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(FREQUENCY)>(leds, NUM_LEDS);
    FastLED.setBrightness(0);
#endif 
}

void loop() {
    
    unsigned long begin, end;
    
    begin = micros();

#ifdef SPI_DMA
    int offset;
    for (int i = 0; i < 128; i++) 
    {
        offset = 4 + (i * 4);
        ledBuffer[offset] = 0xE0 | 0;
        ledBuffer[offset + 1] = 0;  // Random Blue
        ledBuffer[offset + 2] = 255;  // Random Green
        ledBuffer[offset + 3] = 0;  // Random Red 
    }

    updateLEDs();  // Update the LED strip
#else
    for (CRGB led : leds)
        led = CRGB(0, 255, 0);
        
    FastLED.show();
    
#endif

    end = micros();
    
    delay(50);
    
    ESP_LOGI("", "Time took: %dms\n", end - begin);
}