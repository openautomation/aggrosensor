#include "SensorEntry.h"

int read_tmp_env(const struct SensorEntry *entry)
{
  char pinDigital = entry->pins[0];
  char pinAnalog = entry->pins[1];
  pinMode(pinDigital, OUTPUT);
  digitalWrite(pinAnalog, LOW); 
  digitalWrite(pinDigital, HIGH);
  delay(2); 
  float v_out = analogRead(pinAnalog);
  digitalWrite(pinDigital, LOW);
  
  v_out *= ReferenceVoltageMultiplier;
  float temp = 0.0512 * 1000 * v_out - 20.5128;
  
  entry->packageDataMessage(temp);
  return 0;  //no error
}
