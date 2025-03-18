

#include <Arduino.h>

#define HAL_PIN 10
#define SERIAL_BAUDRATE 115200

void setup() 
{
  Serial.begin(SERIAL_BAUDRATE);
  pinMode(HAL_PIN, INPUT);
}

void loop() 
{
  while (true)
  {
    Serial.println(digitalRead(HAL_PIN));
    
    vTaskDelay(pdTICKS_TO_MS(100));
  }
}

