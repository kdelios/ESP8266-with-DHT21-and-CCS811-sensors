/* Bedroom environmental data station build, using a ESP8266 NodeMCUV3, and the following sensors:
   
   1. DHT21 (AM2301) sensor measuring temperature (°C) & humidity (%RH).
   
   2. CJMCU-811 CS811 (I2C) sensor measuring eCO2 (the equivalent CO2 *400ppm to 8192ppm*)& TVOC (Total Volatile Organic Compound *0ppb to 1187ppb*).
      CCS811 receives temperature and humidity readings from DHT21 for compensation algorithm.
      New CCS811 sensors require 48h-burn in. Once burned in a sensor requires 20 minutes of run-in before readings are considered good.
      **Connect nWAKE sensor pin directly to GND, so the CCS811 will avoid enter into SLEEP mode [sensor it's always ACTIVE]**
      
   Logging all values to a ThinkSpeak channel every 2 min.
   Build by Konstantinos Deliopoulos @ Dec 2019 */

#include "DHT.h"
#include "Adafruit_CCS811.h"
#include <Adafruit_Sensor.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>       //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <Wire.h>              // Wire library for I2C protocol

const char* host = "api.thingspeak.com";
const char* THINGSPEAK_API_KEY = "YOUR_API_KEY";
const int UPDATE_INTERVAL_SECONDS = 600; // Update post to ThingSpeak every 120 seconds = 2 minutes (120000 ms). Min with ThingSpeak is ~15 seconds for one channel use

Adafruit_CCS811 ccs;                     // CCS811 is connected (I2C) to D1-->SLC-->GPIO5-->Pin5 & D2-->SDA-->GPIO4-->Pin4

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11      // DHT 11
//#define DHTTYPE DHT22      // DHT 22  (AM2302), AM2321

#define DHTTYPE DHT21              // DHT 21 (AM2301)
#define DHTPIN 2                   // Digital pin connected to the DHT sensor yellow wire --> D4 --> GPIO2
#define I2C_CCS811_ADDRESS 0x5A    // CCS811 I2C address
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
  
  Wire.begin();                           // Enable I2C
  dht.begin();                            // Enable DHT21
  ccs.begin();                            // Enable CCS811

 Serial.println("CCS811 test");
 if(!ccs.begin()){
 Serial.println("Failed to start CCS811 sensor! Please check your wiring!");
 while(1);

  // Set CCS811 to Mode 2: Pulse heating mode IAQ measurement every 10 seconds
  ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);

}
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

   // Pass DHT21 temp & hum readings to CSS811 for compensation algorithm
   ccs.setEnvironmentalData(t, h);

if(ccs.available()){
float temp = ccs.calculateTemperature();
if(!ccs.readData()){
}
 }
   // Read CCS811 values
  float eco2 = ccs.geteCO2();
  float tvoc = ccs.getTVOC();
  
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
  Serial.print(F("  eCO2: "));
  Serial.print(eco2);
  Serial.println(F(" ppm "));
  Serial.print(F("  TVOC: "));
  Serial.print(tvoc);
  Serial.println(F(" ppb "));

    // Create a URI for the ThingSpeak.com request
    String url = "/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field1=";
    url += String(dht.readTemperature()); // bedroom temperature in Deg C (via DHT21)
    url += "&field2=";
    url += String(dht.readHumidity());    // bedroom humidity in %RH (via DH21)
    url += "&field3=";
    url += String(WiFi.RSSI());            //esp8266 rssi in dbm
    url += "&field4=";
    url += String(ccs.geteCO2());         // bedroom eCO2 (the equivalent CO2 *400ppm to 8192ppm*) (via CCS811)
    url += "&field5=";
    url += String(ccs.getTVOC());         // bedroom TVOC (Total Volatile Organic Compound *0ppb to 1187ppb*) (via CCS811)

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
