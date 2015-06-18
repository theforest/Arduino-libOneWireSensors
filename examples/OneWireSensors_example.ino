// Example on how to use OneWireSensors library
// Requires OneWire library
#include <OneWire.h>
#include <OneWireSensors.h>

OneWire ow(5);
OneWireSensors ows;

void setup() {
  Serial.begin(115200);
}

void loop() {
  #define SENSORS 4
  int temps[SENSORS];
  uint16_t ids[SENSORS];
  int sensorsReturned = 0;
  
  sensorsReturned = ows.getOWTemps(ow, temps, ids, SENSORS);
  
  if(sensorsReturned == 0) Serial.println("No sensors found!");
  
  for(int i = 0; i<sensorsReturned; i++) {
    Serial.print("Sensor:");
    Serial.print(ids[i], HEX);
    Serial.print(" Temp:");
    Serial.println(temps[i]);
  }
}

