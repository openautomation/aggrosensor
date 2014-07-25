#include "../SensorEntry.h"

float read_Tmp_Env(SensorEntry *config){
  char pinDigital = config->pins[0];
  char pinAnalog = config->pins[1];
  pinMode(pinDigital, OUTPUT);
  digitalWrite(pinAnalog, LOW); 
  digitalWrite(pinDigital, HIGH);
  delay(2); 
  float v_out = analogRead(pinAnalog); 
  digitalWrite(pinDigital, LOW);
  v_out *= .0048; 
  v_out *= 1000; 
  float temp = 0.0512 * v_out -20.5128;
  return temp;
} 