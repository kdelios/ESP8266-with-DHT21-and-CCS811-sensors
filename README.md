Bedroom environmental data station build, using a ESP8266 NodeMCUV3, and the following sensors:
   
   1. DHT21 (AM2301) sensor measuring temperature (°C) & humidity (%RH).
   
   2. CJMCU-811 CS811 (I2C) sensor measuring eCO2 (the equivalent CO2 *400ppm to 8192ppm*)& TVOC (Total Volatile Organic Compound *0ppb to 1187ppb*).
      CCS811 receives temperature and humidity readings from DHT21 for compensation algorithm.
      New CCS811 sensors require 48h-burn in. Once burned in a sensor requires 20 minutes of run-in before readings are considered good.
      **Connect nWAKE sensor pin directly to GND, so the CCS811 will avoid enter into SLEEP mode [sensor it's always ACTIVE]**
      
   Logging all values to a ThinkSpeak channel every 2 min.
   Build by Konstantinos Deliopoulos @ Dec 2019
