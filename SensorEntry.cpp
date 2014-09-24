#include "SensorEntry.h"
#include "SensorManager.h"

#include <Arduino.h>

SensorMeasurementFunc SensorEntry::getMeasurementFunc() const {
  return func;
}

const char* SensorEntry::getSensorID() const {
  return idFromSensorFunc(func);
}

int SensorEntry::writeToEEPROM(int address) {
  // We must write the sensorID, instead of the function address, as we want settings to survive flashing the board
  // to a new version, and measurement function addresses will probably change between versions.
  // So copy in the string, which has a maximum length, at the end (func is last for this reason).
  const int ID_OFFSET = offsetof(SensorEntry, func);
  memcpyToEEPROM(address, this, ID_OFFSET);
  strcpyToEEPROM(address + ID_OFFSET, getSensorID());
  return ID_OFFSET + MAX_SENSOR_ID_LENGTH;
}

int SensorEntry::readFromEEPROM(int address) {
  // see comments in writeToEEPROM()

  // read everything but the ID
  const int ID_OFFSET = offsetof(SensorEntry, func);
  memcpyFromEEPROM(address, this, ID_OFFSET);
  
  // decode the sensor ID into a function pointer
  char id[MAX_SENSOR_ID_LENGTH];
  memcpyFromEEPROM(address + ID_OFFSET, id, MAX_SENSOR_ID_LENGTH);
  func = funcFromSensorID(id);
  
  return ID_OFFSET + MAX_SENSOR_ID_LENGTH;
}

///////////////////////////////////////////////////////////////////////////////////////////
// send data message
void SensorEntry::packageDataMessage(float data) const {
  Serial.print(F("{\"measurement\":{\"label\":\""));
  Serial.print(label);
  Serial.print(F("\",\"datum\":"));
  Serial.print(data);
  Serial.println("}}");
}

void SensorEntry::print(int index) const {
  Serial.print('{');
  
  if (isEmpty()) {
    Serial.print('}');
    return;
  }
  
  if (index >= 0) {
    Serial.print(F("\"index\":"));
    Serial.print(index);
    Serial.print(',');
  }
  
  Serial.print(F("\"label\":\""));
  Serial.print(label);
  Serial.print(F("\",\"sensorID\":\""));
  Serial.print(getSensorID());
  Serial.print(F("\",\"msMeasurementPeriod\":"));
  Serial.print(msMeasurementPeriod);
  Serial.print(F(",\"pins\":["));
  for(int i = 0; i < NUM_PINS; i++) {
    if (pins[i] == -1) break;            // assume no more pins after a -1
    if (i != 0) Serial.print(',');
    Serial.print((int)pins[i]);
  }
  Serial.print("]}");
}
