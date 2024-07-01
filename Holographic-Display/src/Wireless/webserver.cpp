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
  
  size_t totalBytes, usedBytes, freeBytes;
  totalBytes = SPIFFS.totalBytes();
  usedBytes = SPIFFS.usedBytes();
  freeBytes = totalBytes - usedBytes;

  Serial.print(F("SPIFFS Free: ")); 
  Serial.println(_format_bytes(freeBytes));
  Serial.print(F("SPIFFS Used: ")); 
  Serial.println(_format_bytes(usedBytes));
  Serial.print(F("SPIFFS Total: ")); 
  Serial.println(_format_bytes(totalBytes));

  return true;
}

#ifdef OTA_FIRMWARE
void WebServer::_begin_OTA()
{
  Serial.print(F("Setting up ElegantOTA...")); 

  ElegantOTA.begin(&_server);
  // TODO: setup elegantOTA and stuffs
  Serial.println(F("Done")); 
}
#endif

#ifdef MDNS_HOSTNAME
bool WebServer::_begin_mDNS()
{
  Serial.print(F("Setting up mDNS...")); 

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    Serial.println(F("An Error has occurred while setting up MDNS responder!"));
    return false;
  }

  Serial.println(F("Done")); 

  return true;
}
#endif

void WebServer::_setup_webserver_tree()
{
  Serial.print(F("Setting up server tree...")); 

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site/index.html", "text/html");
  });

  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site/style.css", "text/css");
  });

  _server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site/script.js", "application/javascript");
  });

  _server.on("/rgb.gif", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site/rgb.gif", "image/gif");
  });

  _server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());
    request->send(SPIFFS, "/site/404.html", "text/html");
  });

  _server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
  });

_server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    Serial.println("upload!");
    if (!index) {
      Serial.printf("UploadStart: %s\n", filename.c_str());
      request->_tempFile = SPIFFS.open("/" + filename, "w");
    }

    size_t REMAINING_BYTES = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    Serial.println(REMAINING_BYTES);
    
    if (request->_tempFile) {
      if (index + len > REMAINING_BYTES) {
        request->_tempFile.close();
        return;
      }

      request->_tempFile.write(data, len);
    }
    
    if (final) {
      if (request->_tempFile) {
        request->_tempFile.close();
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
      } else {
        Serial.printf("Upload failed: %s exceeds maximum size of %u bytes\n", filename.c_str(), REMAINING_BYTES);
      }
    }
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
   _server.begin();

  Serial.print(F("Done")); 
}

}