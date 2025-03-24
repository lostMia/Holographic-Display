



#include <Arduino.h>

#define HAL_PIN 20
#define SERIAL_BAUDRATE 115200

void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);

  printf("setup done\n");
  pinMode(HAL_PIN, INPUT);
}

void loop() 
{
  while (true)
  {
    printf("%d\n", digitalRead(HAL_PIN));
    
    vTaskDelay(pdTICKS_TO_MS(100));
  }
}

