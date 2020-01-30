/* Bedroom environmental data station build, using a ESP01, and the following sensor:
   
   1. DHT21 (AM2301) sensor measuring temperature (°C) & humidity (%RH).
    
   Logging all values to a ThinkSpeak channel every 2 min.
   Build by Konstantinos Deliopoulos @ Jan 2020 */

#include "DHT.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>       //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>

const char* host = "api.thingspeak.com";
const char* THINGSPEAK_API_KEY = "YOUR_API_KEY";  // MY CHANNEL 2 ****Back room****
const int UPDATE_INTERVAL_SECONDS = 600; // Update post to ThingSpeak every 120 seconds = 2 minutes (120000 ms). Min with ThingSpeak is ~15 seconds for one channel use

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11      // DHT 11
//#define DHTTYPE DHT22      // DHT 22  (AM2302), AM2321

#define DHTTYPE DHT21              // DHT 21 (AM2301)
#define DHTPIN 2                   // Digital pin connected to the DHT sensor yellow wire --> GPIO2
#define LED_PIN D4                 // GPIO2 == D4 == standard BLUE led available on most NodeMCU boards (LED on == D4 low)
#define led_init() pinMode(LED_PIN, OUTPUT)
#define led_on()   digitalWrite(LED_PIN, LOW)
#define led_off()  digitalWrite(LED_PIN, HIGH)
#define led_tgl()  digitalWrite(LED_PIN, (HIGH+LOW)-digitalRead(LED_PIN));

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  
  // Enable serial
  delay(3000);   // Give user some time to connect USB serial
  Serial.begin(115200);
  
  WiFiManager wifiManager;
   //reset saved settings
   //wifiManager.resetSettings();
  wifiManager.autoConnect("AutoConnectAP");
  
  dht.begin();    // Enable DHT21
}
   
void loop() {
  
  // Wait a few seconds between measurements
  delay(2000);

  // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed!");
      return;
    }

  /* Reading temperature or humidity takes about 250 milliseconds!
     Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)*/
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

   // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

   // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
   // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print(F("  Temperature: "));
  Serial.print(t);
  Serial.println(F(" °C "));
  Serial.print(F("  Humidity: "));
  Serial.print(h);
  Serial.println(F(" %RH "));
  Serial.print(F("  Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  Serial.print("  RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println("dbm");

    // Create a URI for the ThingSpeak.com request
    String url = "/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field6=";
    url += String(dht.readTemperature()); // back room temperature in Deg C (via DHT21)
    url += "&field7=";
    url += String(dht.readHumidity());    // back room humidity in %RH (via DH21)
    url += "&field8=";
    url += String(WiFi.RSSI());           // back room esp8266 rssi in dbm
   
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // Send the request to the ThinkSpeak server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");             
 delay(10);
    while(!client.available()){
      delay(100);
      Serial.print(".");
    }
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection..."); 

  delay(200 * UPDATE_INTERVAL_SECONDS);
}
