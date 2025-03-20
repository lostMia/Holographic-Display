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
    request->send(LittleFS, F("/site/main/index.html"), F("text/html"));
  });
  
  _server.on(PSTR("/TargetPower"), HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    char buffer[10];
    sprintf(buffer, "%d", _target_power);

    request->send(200, F("text/plain"), buffer);
  });

  _server.on(PSTR("/CanUpload"), HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    char buffer[10];
    sprintf(buffer, "%d", _can_upload);

    request->send(200, F("text/plain"), buffer);
  });
 
  _server.on(PSTR("/CurrentRPM"), HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    char buffer[10];
    sprintf(buffer, "%d", _current_RPM);
    request->send(200, F("text/plain"), buffer);
  });
  
  _server.onNotFound([](AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "Unable to find http://%s | request from %s\n", request->host().c_str(), request->client()->remoteIP().toString().c_str());

    request->send(LittleFS, F("/site/notfound/index.html"), F("text/html"));
  });

  _server.onFileUpload([this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) 
  {
    // If a new upload has been started.
    if (!index)                   
    {
      ESP_LOGI(TAG, "Upload started!");
      ESP_LOGI(TAG, "DMO Mode: %s", _dmo_mode ? "enabled" : "disabled");
                       
      if (!_dmo_mode)
        request->_tempFile = LittleFS.open(IMAGE_DATA_NAME, "w");

      _frame_buffer_index = 0;
      _frame_counter = 0;
    }
                       
    // Copy the received data into the frame buffer.
    memcpy(_frame_buffer + _frame_buffer_index, 
      data,
      len
    );

    _frame_buffer_index += len;
       
    // If the frame buffer is full.
    if (_frame_buffer_index >= IMAGE_SIZE_BYTES + 2)
    {
      _renderer->update_frame(_frame_counter, _frame_buffer);
                 
      // Only care about writing anything to the file system if we aren't in DMU mode!
      if (!_dmo_mode)
      {
        size_t free_bytes = LittleFS.totalBytes() - LittleFS.usedBytes();
        ESP_LOGI(TAG, "LittleFS Free: %s", _format_bytes(free_bytes).c_str());
                       
        if (request->_tempFile)
          request->_tempFile.write(_frame_buffer, IMAGE_SIZE_BYTES + 2);
        }

      size_t remaining_bytes = _frame_buffer_index - (IMAGE_SIZE_BYTES + 2);

      // Move the rest of the buffer back to the start if there is remaining data.
      if (remaining_bytes > 0)
        memmove(_frame_buffer,
            _frame_buffer + (IMAGE_SIZE_BYTES + 2),
            remaining_bytes
        );
  
      _frame_buffer_index -= (IMAGE_SIZE_BYTES + 2);
      _frame_counter++;
    }
                       
    if (!_dmo_mode && final)
    {
      request->_tempFile.close();
      ESP_LOGI(TAG, "Upload ended! %s, %u\n", filename.c_str(), _format_bytes(index + len));
      ESP_LOGI(TAG, "Free Heap: %d", ESP.getFreeHeap());
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

  _server.serveStatic(PSTR("/resources/"), LittleFS, PSTR("/site/resources/"));
  _server.serveStatic(PSTR("/notfound/"), LittleFS, PSTR("/site/notfound/"));
  _server.serveStatic(PSTR("/datadump/"), LittleFS, PSTR("/datadump/"));
  _server.serveStatic(PSTR("/"), LittleFS, PSTR("/site/main/")).setDefaultFile(PSTR("index.html"));
  _server.begin();
}

// This handles any responses we get from the User-Interface.
void WebServer::_handle_input(const AsyncWebParameter* parameter)
{
  const char* name;
  const char* value;
  uint8_t index = 0;

  try
  {
    // The name will be something like s5 -> Slider 5.
    name = parameter->name().c_str();
    value = parameter->value().c_str();
    index = std::stoi(parameter->name()
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
      switch (index)
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
        // LED Brightness Slider
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
        // Offset-slider 
        case 6:
          _renderer->options.offset = std::stoi(value);
          break;
        default:
          break;
      }
      break;

    // Motor-Speed response 
    case 'm':
      unsigned long delay_per_pulse_us, delay_per_rotation_us;
      float frequency_hz;

      delay_per_pulse_us = std::stoi(value);

      // ESP_LOGI(TAG, "delay_per_pulse_us %d", delay_per_pulse_us);
      
      // If the motor is standing still or the delay is impossibly small.
      if (delay_per_pulse_us == LONG_MAX || delay_per_pulse_us < 1000)
      {
        taskENTER_CRITICAL(&Rendering::optionsMUX);
        // Calculate the time between each degree in μs.
        _renderer->options._delay_between_degrees_us = 5000; // We don't really care since the motor is stuck anyway.

        // Calculate the RPM.
        _current_RPM = 0;
        taskEXIT_CRITICAL(&Rendering::optionsMUX);
      }
      else
      {
        // 9 Pulses for each rotation before the gearbox with a ration of 1 to 10 -> 90 pulses per rotation.
        delay_per_rotation_us = delay_per_pulse_us * 90;
        frequency_hz = 1000000.0 / (float)(delay_per_rotation_us); 

        taskENTER_CRITICAL(&Rendering::optionsMUX);
        // Calculate the time between each degree in μs.
        // ESP_LOGI(TAG, "changing delay to %d", (unsigned long)(delay_per_pulse_us / 4));
      
        _renderer->options._delay_between_degrees_us 
          = (unsigned long)((float)(delay_per_pulse_us) / (8.0 * MAGIC_VALUE_TM));

        // Calculate the RPM.
        _current_RPM = (unsigned long)(frequency_hz * 60.0);
        taskEXIT_CRITICAL(&Rendering::optionsMUX);
      }
      break;

     // Lever-Field
    case 'l':
      // Figure out what lever was used.
      switch (index)
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
          
        // DMU-Mode-Lever
        case 3:
          if (!strncmp(value, "true", 8))
            _dmo_mode = true;
          else
            _dmo_mode = false;
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
  #ifdef OTA_FIRMWARE
  _begin_OTA();
  #endif
  
  #ifdef MDNS_HOSTNAME
  _begin_mDNS();
  #endif
 
  _setup_webserver_tree();
}

} // Namespace Webserver 