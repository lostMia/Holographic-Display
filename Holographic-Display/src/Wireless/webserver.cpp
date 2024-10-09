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

uint16_t RPM_MOTOR = 0;

namespace Wireless
{

WebServer::WebServer(uint16_t port) : _server(port)
{}

String WebServer::_format_bytes(const size_t bytes) 
{
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / (1024.0 * 1024.0)) + " MB";
  else return String(bytes / (1024.0 * 1024.0 * 1024.0)) + " GB";
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

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.print(F("Serving to IP: "));
    Serial.println(request->client()->remoteIP().toString());
    request->send(SPIFFS, "/site/index.html", "text/html");
  });
  
  _server.on("/RPM", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    char RPM_string[10];
    
    sprintf(RPM_string, "%d", RPM_MOTOR);

    request->send(200, "text/plain", RPM_string);
  });

  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/site/style.css", "text/css");
  });

  _server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/site/script.js", "application/javascript");
  });

  _server.on("/rgb.gif", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/site/rgb.gif", "image/gif");
  });

  _server.onNotFound([](AsyncWebServerRequest *request)
  {
    Serial.printf("Unable to find http://%s%s\n", request->host().c_str(), request->url().c_str());
    request->send(SPIFFS, "/site/404.html", "text/html");
  });

  _server.onFileUpload([this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if (!index) 
    {
      Serial.printf("UploadStart: %s\n", filename.c_str());
      request->_tempFile = SPIFFS.open("/" + filename, "w");
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
        Serial.printf("UploadEnd: %s, %u\n", filename.c_str(), _format_bytes(index + len));
      } else {
        Serial.printf("Upload failed: %s exceeds maximum size of %u bytes\n", filename.c_str(), free_bytes);
      }
    }
  });

  _server.on("/post", HTTP_POST, [this](AsyncWebServerRequest *request)
  {
    uint8_t params = request->params();
             
    for(uint8_t i = 0; i < params; i++)
    {
      const AsyncWebParameter* parameter = request->getParam(i);
            
      _handle_input(parameter);
    }
      
    request->send(200, "text/plain", "OK");
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
          RPM_MOTOR = std::stoi(value);
          Serial.println(RPM_MOTOR);
      
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
      // todo: show the user what the speed of the motor is at the moment.
      Serial.println(value);
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

  _server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  _server.begin();

  Serial.println(F("Done"));
}

} // Namespace Wireless