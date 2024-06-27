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

WiFiClientSecure *secureClient = new WiFiClientSecure;

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

void Server::begin() 
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  WiFiClientSecure secureClient;

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  _server.begin();
  Serial.println("HTTPS server started");
}


} // namespace Web