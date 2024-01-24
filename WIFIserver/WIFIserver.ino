#include <ESP8266WiFi.h>

#ifndef STASSID
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

void setup() {
  Serial.begin(115200);
  Serial.print(F("start"));
  // prepare LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("NodeMCU v3");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   Serial.print(F("."));
  }

  // Start the server
  server.begin();
  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client) {   return; }

  client.setTimeout(1000);  // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
 
  // Match the request
  int val;
  if (req.indexOf(F("/parameters/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/parameters/1")) != -1) {
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
  client.print(F("Wojciech Wolosz - Stacja monitoringu parametrow roslin <br> <br> "));
  client.print(F("Measured temperature: "));
  client.print(temperature);
  client.print(F(" Celsius degrees <br>"));
  
  client.print(F("Measured pressure: "));
  client.print(pressure) ;
  client.print(F(" hPa <br>"));

  client.print(F("Measured insolation: "));
  client.print(insolation);
  client.print(F("<br>"));

  client.print(F("Measured moisture of the dirt: "));
  client.print(insolation);
  client.print(F("<br>"));


 client.print(F("Additional info: <br>"));
  if(is_raining)
  {
  client.print(F("Rain detected <br>"));
  }
  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  //Serial.println(F("Disconnecting from client"));
}

