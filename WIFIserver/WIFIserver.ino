/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
// #define STASSID "AGH_Racing"
// #define STAPSK "AGHracing!"
#define STASSID "RetardedRouter"
#define STAPSK "recpixa5"

#endif

const char* ssid = STASSID;
const char* password = STAPSK;

int temperature, pressure, moisture, insolation;
bool is_raining;


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
uint32_t test, test2;
String test_string;
char readData[10];





void setup() {
  Serial.begin(115200);
  Serial.print(F("start"));
  // prepare LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // Connect to WiFi network
  // Serial.println();
  // Serial.println();
  // Serial.print(F("Connecting to "));
  // Serial.println(ssid);




  WiFi.mode(WIFI_STA);
  WiFi.hostname("NodeMCU v3");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   Serial.print(F("."));
  }
  // Serial.println();
  // Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  //Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client) {   return; }
  //Serial.println(F("new client"));


if (Serial.available() > 4) 
{
  test_string = Serial.readBytes(readData,5);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  
}

  client.setTimeout(1000);  // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  //Serial.println(F("request: "));
  //Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/roslinki/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/roslinki/1")) != -1) {
    val = 1;
  } else {
    //Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }

  

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n"));

  client.print(F(""));
  client.print(F("Measured pemperature: "));
  
  client.print(F(" Celsius degrees <br>"));
  
  client.print(F("Measured pressure: "));
  client.print(test);
  client.print(F(" hPa <br>"));

  client.print(F("Measured insolation: "));
  client.print(evaluateInsolation(insolation));
  client.print(F("<br>"));

  client.print(F("Measured moisture of the dirt: "));
  client.print(evaluateInsolation(insolation));
  client.print(F("<br>"));



  client.print(F("Is raining: "));
  client.print(is_raining);
  client.print(F(" <br>"));

  
  client.print(F("uart data:  "));
  client.print(test_string);
   client.print(F(" <br>"));

  client.print((val) ? F("high") : F("low"));
  client.print(F("<br><br>Click <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/roslinki/1'>here</a> to switch LED GPIO on, or <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/roslinki/0'>here</a> to switch LED GPIO off.</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  //Serial.println(F("Disconnecting from client"));
    test++;
}

String evaluateInsolation(uint32_t _value)
{
  //TODO PRZYPISAĆ ARBITRALNE ZALEZNOŚCI
String ret;
ret = "TODO";

return ret;
}

String evaluateMoisture(uint32_t _value)
{
  //TODO PRZYPISAĆ ARBITRALNE ZALEZNOŚCI
String ret;
ret = "TODO";

return ret;
}
