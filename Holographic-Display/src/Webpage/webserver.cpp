/*
 * @file webserver.cpp
 * @authors Timeania, lostmia
 * @brief Defines a Server class for interacting with the webserver.
 * @version 0.1.0
 * @date 2024-05-15
 *
 * Copyright Deimo Elektronik GmbH (c) 2024
*/

#include "Webpage/webserver.hpp"

namespace Web
{

Server::Server(uint16_t port) : _server(port)
{}

void Server::_add_to_json_string(String* variable_string, String* parameter, float* value)
{
  String parameter_string = *parameter;
  String value_string = String(*value, 2);

  *variable_string += "\"" + parameter_string + "\":" + value_string + ",";
}

static String server_ui_size(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }

void Server::begin() 
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Initialize mDNS
  if (!MDNS.begin("holo")) 
  {
    Serial.println("Error setting up MDNS responder!");
    while(1) 
    {
      delay(1000);
    }
  }

  Serial.print("SPIFFS Free: "); Serial.println(server_ui_size((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  Serial.print("SPIFFS Used: "); Serial.println(server_ui_size(SPIFFS.usedBytes()));
  Serial.print("SPIFFS Total: "); Serial.println(server_ui_size(SPIFFS.totalBytes()));
  
  
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  _server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "application/javascript");
  });

  _server.on("/rgb.gif", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/rgb.gif", "image/gif");
  });
  
   _server.begin();
  //_server.beginSecure(cert_string, key_string, NULL);
  //_server.beginSecure(certFile.readString().c_str(), keyFile.readString().c_str(), NULL);
}

} // namespace Web