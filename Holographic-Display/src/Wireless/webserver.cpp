/*
 * @file webserver.cpp
 * @authors mia
 * @brief Defines a Server class for interacting with the webserver.
 * @version 0.1.0
 * @date 2024-05-15
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 72
// Note: this can be 12 if you're using a teensy 3 and don't mind soldering the pads on the back
#define NUM_STRIPS 2

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

uint32_t color_number;

#include "Wireless/webserver.hpp"

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
    Serial.printf("http://%s%s\n", request->host().c_str(), request->url().c_str());
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

  _server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    int params = request->params();

    for(int i=0;i<params;i++)
    {
      const AsyncWebParameter* p = request->getParam(i);
      Serial.print(p->name().c_str());
      Serial.print(": ");
      Serial.println(p->value().c_str());
      Serial.println(p->value().substring(2).c_str());
             
      std::string color_string = p->value().substring(2).c_str();
      
      color_number = std::stoi(color_string, 0, 16);

      Serial.println(color_number);
    }
      
    request->send(200, "text/plain", "OK");
  });

  Serial.println(F("Done")); 
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

  Serial.println("Starting... the led's now..");
  FastLED.addLeds<NEOPIXEL, 27>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS);

  uint8_t hue = 0;
  while (true)
  {
    for (int i = 0; i < NUM_LEDS_PER_STRIP * NUM_STRIPS; i++)
    {
      leds[i] = CRGB::Red;
      leds[i].setColorCode(color_number);
      leds[i].nscale8(255);
    }

    FastLED.show();
  }
}

} // Namespace Wireless