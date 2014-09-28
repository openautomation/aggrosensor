#include "SensorEntry.h"

int read_DFRobot_pH_Sensor(const SensorEntry *entry)
{
  char sensorPin = entry->pins[0];
  char ledPin = entry->pins[1];
  
  float phValue = analogRead(sensorPin);
  //Serial.print(phValue);
  phValue *= ReferenceVoltageMultiplier / 6;  //convert the analog into millivolt
  phValue *= 3.5;                             //convert the millivolt into pH value
  entry->packageDataMessage(phValue);

  // flash the LED
  pinMode(ledPin, OUTPUT);  
  digitalWrite(ledPin, HIGH);       
  delay(200);
  digitalWrite(ledPin, LOW); 

  return 0;  //no error
}
