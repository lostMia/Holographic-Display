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

uint16_t target_speed = 0;
uint8_t led_brightness = 100;
uint16_t current_speed = 0;

unsigned long delay_between_frames_ms = 10;

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
  Serial.print(F("Setting up ElegantOTA...")); 

  ElegantOTA.begin(&_server);
  ElegantOTA.setAutoReboot(true);

#ifdef OTA_USERNAME
#ifdef OTA_PASSWORD
  ElegantOTA.setAuth(OTA_USERNAME, OTA_PASSWORD);
#endif
#endif

  Serial.println(F("Done")); 
}
#endif

#ifdef MDNS_HOSTNAME
bool WebServer::_begin_mDNS()
{
  Serial.print(F("Setting up mDNS hostname...")); 

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    Serial.println(F("An Error has occurred while setting up mDNS responder!"));
    return false;
  }

  Serial.println(F("Done")); 
  Serial.print(F("Setting up mDNS Service...")); 

  if (!MDNS.addService("http", "tcp", 80))
  {
    Serial.println(F("An Error has occurred while setting up mDNS Service!"));
    return false;
  }

  Serial.println(F("Done")); 

  return true;
}
#endif

void WebServer::_setup_webserver_tree()
{
  Serial.print(F("Setting up server tree...")); 

  _server.on(PSTR("/"), HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.print(F("Serving to IP: "));
    Serial.println(request->client()->remoteIP().toString());
    request->send(SPIFFS, F("/site/main/index.html"), F("text/html"));
  });
  
  _server.on(PSTR("/TargetRPM"), HTTP_GET, [](AsyncWebServerRequest *request)
  {
    char RPM_string[10];
    
    sprintf(RPM_string, "%d", target_speed);

    request->send(200, F("text/plain"), RPM_string);
  });
 
  _server.on(PSTR("/CurrentRPM"), HTTP_GET, [](AsyncWebServerRequest *request)
  {
    char RPM_string[10];
    
    sprintf(RPM_string, "%d", current_speed);

    request->send(200, F("text/plain"), RPM_string);
  });
  
  _server.onNotFound([](AsyncWebServerRequest *request)
  {
    Serial.printf(PSTR("Unable to find http://%s%s\n"), request->host().c_str(), request->url().c_str());
    request->send(SPIFFS, F("/site/notfound/index.html"), F("text/html"));
  });

  _server.onFileUpload([this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) 
  {
    if (!index) 
    {
      Serial.printf(PSTR("UploadStart: %s\n"), filename.c_str());
      request->_tempFile = SPIFFS.open(IMAGE_JSON_NAME, "w");
    }

    size_t free_bytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    Serial.print(F("SPIFFS Free: ")); 
    Serial.println(_format_bytes(free_bytes));
      
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
        Serial.printf(PSTR("UploadEnd: %s, %u\n"), filename.c_str(), _format_bytes(index + len));

        Serial.print(F("Free Heap:"));
        Serial.println(ESP.getFreeHeap());
          
        _renderer->load_image_data();
      } 
      else
      {
        Serial.printf(PSTR("Upload failed: %s exceeds maximum size of %u bytes\n"), filename.c_str(), free_bytes);
      }
    }
  });

  _server.on(PSTR("/post"), HTTP_POST, [this](AsyncWebServerRequest *request)
  {
    Serial.println(F("accessing /post..."));
    uint8_t params = request->params();
    
    for(uint8_t i = 0; i < params; i++)
    {
      const AsyncWebParameter* parameter = request->getParam(i);
            
      _handle_input(parameter);
    }
      
    request->send(200, F("text/plain"), F("OK"));
  });

  Serial.println(F("Done")); 
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
    Serial.println(F("Failed to parse input!"));
    return;
  }
  
  // Figure out, what type of element sent the response
  switch (name[0])
  {
    // Slider
    case 's':
      // Figure out what slider was used
      switch (number)
      {
        // RPM-Slider
        case 1:
          target_speed = std::stoi(value);
          break;
        // LED-Slider
        case 2:
          led_brightness = std::stoi(value);
          FastLED.setBrightness(led_brightness);
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

    // Radio-Button
    case 'r':
      break;

    // RPM-Motor response 
    case 'm':
        current_speed = std::stoi(value);

        // Get the current rpm -> rps -> delay between every rotation -> delay between every degree
        // delay_between_frames_ms;

        // todo: fix this. find out, if we should transmit the rpm or just the last delay between rotations....
      break;

    // Text-Field
    case 't':
      break;

    // Color-Input
    case 'c':
      std::string color_string = parameter->value().substring(2).c_str();

      Serial.println(color_string.c_str());
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

  Serial.print(F("Server starting...")); 

  _server.serveStatic(PSTR("/resources/"), SPIFFS, PSTR("/site/resources/"));
  _server.serveStatic(PSTR("/notfound/"), SPIFFS, PSTR("/site/notfound/"));
  _server.serveStatic(PSTR("/datadump/"), SPIFFS, PSTR("/datadump/"));
  _server.serveStatic(PSTR("/"), SPIFFS, PSTR("/site/main/")).setDefaultFile(PSTR("index.html"));
  _server.begin();

  _renderer->init(&delay_between_frames_ms);
  
  Serial.println(F("Done"));
}

} // Namespace Wireless