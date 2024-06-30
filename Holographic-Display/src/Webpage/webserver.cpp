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

const char* cert_string =
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIB/jCCAWegAwIBAgIUKYo525MIJrfmw/4Cy+QuLniLfX8wDQYJKoZIhvcNAQEL\n" \ 
"BQAwETEPMA0GA1UEAwwGaXNzdWVyMB4XDTI0MDYzMDE1NTA1MVoXDTI1MDYzMDE1\n" \ 
"NTA1MVowETEPMA0GA1UEAwwGaXNzdWVyMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCB\n" \ 
"iQKBgQCck2cVZjJVlyTLJ114CDq3ejYmcyZjQIqqjjhKEelon0iZHeMlUeYC3Rz0\n" \ 
"BwJe/dHGM1NTMJLSQUUclZ19vh8lgQ9lY3w3BI6IHE825S3+zoHUK9aTwa/MUs10\n" \ 
"Y76VQIiWMlXOcmVAB2nkWw6IK5LotrFKdmddi/vc9ooV8AG3swIDAQABo1MwUTAd\n" \ 
"BgNVHQ4EFgQU1IkgRMFRno+8hfnSs92aD1jBQnswHwYDVR0jBBgwFoAU1IkgRMFR\n" \ 
"no+8hfnSs92aD1jBQnswDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOB\n" \ 
"gQBHZMp+1Zw8R4K/p0G9d/8RWR3XVE9+GCKvGOQMAqHc3UAj8nDK+g6d8z/wWZtS\n" \ 
"pXPIcw8WGfr5QIkIHkBJ+Ig8s42l8pW3VV+Fq6iCJD93EZUMnTdV/mh+3yodqt0J\n" \ 
"6UpcU65MB5u8K4sYXkz14ySdAftbjp05n7+oEud7Eyvayw==\n" \ 
"-----END CERTIFICATE-----\n";

const char* key_string =
"-----BEGIN PRIVATE KEY-----\n" \ 
"MIICeQIBADANBgkqhkiG9w0BAQEFAASCAmMwggJfAgEAAoGBAJyTZxVmMlWXJMsn\n" \ 
"XXgIOrd6NiZzJmNAiqqOOEoR6WifSJkd4yVR5gLdHPQHAl790cYzU1MwktJBRRyV\n" \ 
"nX2+HyWBD2VjfDcEjogcTzblLf7OgdQr1pPBr8xSzXRjvpVAiJYyVc5yZUAHaeRb\n" \ 
"Dogrkui2sUp2Z12L+9z2ihXwAbezAgMBAAECgYEAlsK7a/hmGNGahN4Xep754naT\n" \ 
"MzyGxuR7YDPmcTOD+c/8+Cm179ZG0ZUT7sfc+ZgN53i+D4jipf5gzGAnL87jbxiW\n" \ 
"k4RgHELhyQrrzjWLVdg/kgSGA8sOx8+24/hVTym3yKw4757RRcmaltnaBzVG59PD\n" \ 
"GdwT7al7AAKZvTJuVjECQQDLexO5aYkjEjHxSZ3wu+6GzNgVsfIDZcFTDuKB3Fcj\n" \ 
"qY+xa3RDnGmZVQ3w5nPZhFvMXPwWWEZDf8Is8T1groBbAkEAxP0YV1OjnfJoYWeZ\n" \ 
"N5ajf0KKJSlVGdBPL0ej5urM81yTVE3CFX+hDveAJM4XAk11sfrWdChRS2lBJY8Y\n" \ 
"FNvFiQJBAK+STv6Y1T9xn642exUQ00t0zK5LkCrmTd1A5qhuWrHmyJmpwfPkrikj\n" \ 
"fpfyANwanKrFQK53FImBXyYwMWmOqCkCQQCHXVCbubrmb8xwmss72sUxTx01GikD\n" \ 
"mxS+7aDgrpr15LjNJr3nHhQj/8nMAN7o0ye8jR1PJuFuS77bu3AV7UvZAkEAqS/p\n" \ 
"dzDejd6ogTkLzN1rQMUVuZ4Vz24SyOGzL9uWmQANqs2jaXWNG0hAV/nPLu98Aq1R\n" \ 
"JHjynYemCGe62HVH9g==\n" \ 
"-----END PRIVATE KEY-----\n";

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

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  _server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "application/javascript");
  });

  _server.onSslFileRequest([](void * arg, const char *filename, uint8_t **buf) -> int {
    Serial.printf("SSL File: %s\n", filename);
    File file = SPIFFS.open(filename, "r");
    if(file){
      size_t size = file.size();
      uint8_t * nbuf = (uint8_t*)malloc(size) + 1;
      if(nbuf){
        size = file.read(nbuf, size);
        file.close();
        *buf = nbuf;
        return size;
      }
      file.close();
    }
    *buf = 0;
    return 0;
  }, NULL);

  _server.beginSecure(cert_string, key_string, NULL);
  //_server.beginSecure(certFile.readString().c_str(), keyFile.readString().c_str(), NULL);

  Serial.println("HTTPS server started");

}


} // namespace Web