/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBUS-QLyRl65SxRq17TNWPKeWhTKLaFhe37";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "APT_plus";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "gwp_110701";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL_TEMP_C = "temperature_c"; // Variable label for temperature in Celsius
const char *VARIABLE_LABEL_TEMP_F = "temperature_f"; // Variable label for temperature in Fahrenheit
const char *VARIABLE_LABEL_LDR = "light_intensity"; // Variable label for LDR
const char *VARIABLE_LABEL_BH1750 = "light_intensity_bh1750"; // Variable label for BH1750
const char *VARIABLE_LABEL_PH = "ph_data"; // Variable label for pH data

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

unsigned long timer;
const int oneWireBus = 4;     // GPIO where the DS18B20 is connected to
const int ldrPin = 34;        // GPIO where the LDR is connected to

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

BH1750 lightMeter(0x23); // Initialize BH1750 with default address 0x23

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  Serial.begin(115200);
  sensors.begin();
  Wire.begin(); // Initialize I2C bus for BH1750
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); // Initialize BH1750 in continuous high resolution mode
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

void loop()
{
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  if ((millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    int ldrValue = analogRead(ldrPin);
    float bh1750Lux = lightMeter.readLightLevel(); // Read light intensity from BH1750

    // Generate random pH value between 7.1 and 7.9
    float ph_data = 7.1 + ((float)random(0, 80) / 100); 

    // Send data to Ubidots
    ubidots.add(VARIABLE_LABEL_TEMP_C, temperatureC);
    ubidots.add(VARIABLE_LABEL_TEMP_F, temperatureF);
    ubidots.add(VARIABLE_LABEL_LDR, ldrValue);
    ubidots.add(VARIABLE_LABEL_BH1750, bh1750Lux); // Add BH1750 light intensity data
    ubidots.add(VARIABLE_LABEL_PH, ph_data); // Add pH data
    
    ubidots.publish(DEVICE_LABEL);
    
    // Print data to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print("°C / ");
    Serial.print(temperatureF);
    Serial.print("°F | Light Intensity (LDR): ");
    Serial.print(ldrValue);
    Serial.print(" | Light Intensity (BH1750): ");
    Serial.print(bh1750Lux);
    Serial.print(" lx | pH: ");
    Serial.println(ph_data);

    timer = millis();
  }
  ubidots.loop();
}
