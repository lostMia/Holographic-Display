/*
 * @file webserver.cpp
 * @authors mia
 * @brief Defines a Server class for interacting with the webserver.
 * @version 0.1.0
 * @date 2024-05-15
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "Wireless/webserver.hpp"


namespace Wireless
{

WebServer::WebServer(uint16_t port, Rendering::Renderer *renderer) : _server(port)
{
  _renderer = renderer;
}

String WebServer::_format_bytes(const size_t bytes) 
{
  if (bytes < 1024) 
    return String(bytes) + F(" B");
  else if (bytes < (1024 * 1024)) 
    return String(bytes / 1024.0) + F(" KB");
  else if (bytes < (1024 * 1024 * 1024)) 
    return String(bytes / (1024.0 * 1024.0)) + F(" MB");
  else 
    return String(bytes / (1024.0 * 1024.0 * 1024.0)) + F(" GB");
}

bool WebServer::_begin_SPIFFS()
{
  Serial.print(F("Setting up SPIFFS...")); 

  if (!SPIFFS.begin(true)) 
  {
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
    return false;
  }

  Serial.println(F("Done")); 
  
  size_t total_bytes, used_bytes, free_bytes;
  total_bytes = SPIFFS.totalBytes();
  used_bytes = SPIFFS.usedBytes();
  free_bytes = total_bytes - used_bytes;

  Serial.print(F("SPIFFS Free: ")); 
  Serial.println(_format_bytes(free_bytes));
  Serial.print(F("SPIFFS Used: ")); 
  Serial.println(_format_bytes(used_bytes));
  Serial.print(F("SPIFFS Total: ")); 
  Serial.println(_format_bytes(total_bytes));

  return true;
}

#ifdef OTA_FIRMWARE
void WebServer::_begin_OTA()
{
  ESP_LOGI(TAG, "Setting up ElegantOTA...");

  ElegantOTA.begin(&_server);
  ElegantOTA.setAutoReboot(true);

#ifdef OTA_USERNAME
#ifdef OTA_PASSWORD
  ElegantOTA.setAuth(OTA_USERNAME, OTA_PASSWORD);
#endif
#endif

  ESP_LOGI(TAG, "Done");
}
#endif

#ifdef MDNS_HOSTNAME
bool WebServer::_begin_mDNS()
{
  ESP_LOGI(TAG, "Setting up mDNS hostname...");

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    ESP_LOGE(TAG, "An Error has occurred while setting up mDNS responder!");
    return false;
  }

  ESP_LOGI(TAG, "Setting up mDNS Service...");

  if (!MDNS.addService("http", "tcp", 80))
  {
    ESP_LOGE(TAG, "An Error has occurred while setting up mDNS service!");
    return false;
  }

  return true;
}
#endif

void WebServer::_setup_webserver_tree()
{
  ESP_LOGI(TAG, "Setting up server tree...");

  _server.on(PSTR("/"), HTTP_GET, [](AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "Serving to IP: %s", request->client()->remoteIP().toString().c_str());
    request->send(SPIFFS, F("/site/main/index.html"), F("text/html"));
  });
  
  _server.on(PSTR("/TargetPower"), HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    char RPM_string[10];
    
    sprintf(RPM_string, "%d", _target_power);

    request->send(200, F("text/plain"), RPM_string);
  });
 
  _server.on(PSTR("/CurrentRPM"), HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    char RPM_string[10];
    
    sprintf(RPM_string, "%d", _current_RPM);

    request->send(200, F("text/plain"), RPM_string);
  });
  
  _server.onNotFound([](AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "Unable to find http://%s | request from %s\n", request->host().c_str(), request->client()->remoteIP().toString().c_str());

    request->send(SPIFFS, F("/site/notfound/index.html"), F("text/html"));
  });

  _server.onFileUpload([this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) 
  {
    if (!index) 
    {
      ESP_LOGI(TAG, "UploadStart: %s\n", filename.c_str());
      request->_tempFile = SPIFFS.open(IMAGE_JSON_NAME, "w");
    }

    size_t free_bytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    ESP_LOGI(TAG, "SPIFFS Free: %s", _format_bytes(free_bytes).c_str());
      
    if (request->_tempFile)
    {
      if (index + len > free_bytes)
      {
        request->_tempFile.close();
        return;
      }

      request->_tempFile.write(data, len);
    }
    
    if (final)
    {
      if (request->_tempFile)
      {
        request->_tempFile.close();
        ESP_LOGI(TAG, "UploadEnd: %s, %u\n", filename.c_str(), _format_bytes(index + len));
        ESP_LOGI(TAG, "Free Heap: %d", ESP.getFreeHeap());

        _renderer->refresh_image();
      } 
      else
        ESP_LOGI(TAG, "Upload failed: %s exceeds maximum size of %u bytes\n", filename.c_str(), free_bytes);
    }
  });

  _server.on(PSTR("/post"), HTTP_POST, [this](AsyncWebServerRequest *request)
  {
    uint8_t params = request->params();
    
    for(uint8_t i = 0; i < params; i++)
    {
      const AsyncWebParameter* parameter = request->getParam(i);
            
      _handle_input(parameter);
    }
      
    request->send(200, F("text/plain"), F("OK"));
  });

  _server.serveStatic(PSTR("/resources/"), SPIFFS, PSTR("/site/resources/"));
  _server.serveStatic(PSTR("/notfound/"), SPIFFS, PSTR("/site/notfound/"));
  _server.serveStatic(PSTR("/datadump/"), SPIFFS, PSTR("/datadump/"));
  _server.serveStatic(PSTR("/"), SPIFFS, PSTR("/site/main/")).setDefaultFile(PSTR("index.html"));
  _server.begin();
}

void WebServer::_begin_renderer()
{
  ESP_LOGI(TAG, "Starting the renderer...");

  _renderer->init();
}

// This handles any responses we get from the User-Interface.
void WebServer::_handle_input(const AsyncWebParameter* parameter)
{
  const char* name;
  const char* value;
  uint8_t number = 0;

  try
  {
    // The name will be something like s5 -> Slider 5.
    name = parameter->name().c_str();
    value = parameter->value().c_str();
    number = std::stoi(parameter->name()
                                      .substring(1)
                                      .c_str());
  }
  catch (...)
  {
    ESP_LOGE(TAG, "Failed to parse input!\n");
    return;
  }
  
  // Figure out what type of element sent the response.
  switch (name[0])
  {
    // Slider
    case 's':
      // Figure out what slider was used.
      switch (number)
      {
        // Motor-Power-Slider
        case 1:
          if (!_motor_enabled)
            break;

          float raw_power;
          
          raw_power = (float)std::stoi(value);
          raw_power = raw_power * (255.0 - 96.0) / 100.0 + 96.0;

          _target_power = (uint16_t)raw_power;
          break;
        // LED-Slider
        case 2:
          _led_brightness = std::stoi(value);
          _renderer->set_brightness(_led_brightness);
          break;
        // Red-Color-Slider
        case 3:
          _renderer->options.red_color_adjust = std::stoi(value);
          break;
        // Green-Color-Slider
        case 4:
          _renderer->options.green_color_adjust = std::stoi(value);
          break;
        // Blue-Color-Slider
        case 5:
          _renderer->options.blue_color_adjust = std::stoi(value);
          break;
        default:
          break;
      }
      break;

    // Motor-Speed response 
    case 'm':
      unsigned long delay_per_pulse_us;
      float delay_per_pulse_s, delay_per_rotation_s, frequency_hz;

      delay_per_pulse_us = std::stoi(value);
      
      // If the motor is standing still or the delay is impossibly small.
      if (delay_per_pulse_us == LONG_MAX || delay_per_pulse_us < 1000)
      {
        taskENTER_CRITICAL(&Rendering::optionsMUX);
        // Calculate the time between each degree in μs.
        _renderer->options._delay_between_degrees_us = LONG_MAX; // We don't really care since the motor is stuck anyway.

        // Calculate the RPM.
        _current_RPM = 0;
        taskEXIT_CRITICAL(&Rendering::optionsMUX);
      }
      else
      {
        delay_per_pulse_s = (float)(delay_per_pulse_us) / 1000000.0;
        // 9 Pulses for each rotation before the gearbox with a ration of 1 to 10 -> 90 pulses per rotation.
        delay_per_rotation_s = delay_per_pulse_s * 90;
        frequency_hz = 1.0 / delay_per_rotation_s; 

        taskENTER_CRITICAL(&Rendering::optionsMUX);
        // Calculate the time between each degree in μs.
        _renderer->options._delay_between_degrees_us 
          = (unsigned long)((float)(delay_per_pulse_us) * 90 / 360);

        // Calculate the RPM.
        _current_RPM = (unsigned long)(frequency_hz * 60.0);
        taskEXIT_CRITICAL(&Rendering::optionsMUX);
      }
      break;

    // Text-Field
    case 't':
      break;
      
     // Lever-Field
    case 'l':
      // Figure out what lever was used.
      switch (number)
      {
        // Motor-Active-Lever
        case 1:
          if (!strncmp(value, "true", 8))
          {
            _motor_enabled = true;
          }
          else
          { 
            _motor_enabled = false;
            _target_power = 0;
          }
          break;
        // LED-Active-Lever
        case 2:
          if (!strncmp(value, "true", 8))
            _renderer->set_renderer_state(true);
          else
            _renderer->set_renderer_state(false);
          break;
      }
      break;

    // Color-Input
    case 'c':
      std::string color_string = parameter->value().substring(2).c_str();

      ESP_LOGI(TAG, "Color: %s", parameter->value().substring(2).c_str());
      break;
  }
}

void WebServer::begin() 
{
  if (!_begin_SPIFFS())
    return;

  #ifdef OTA_FIRMWARE
  _begin_OTA();
  #endif
  
  #ifdef MDNS_HOSTNAME
  _begin_mDNS();
  #endif
 
  _setup_webserver_tree();
  
  _begin_renderer();
}

} // Namespace Webserver 