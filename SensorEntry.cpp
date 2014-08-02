#include "SensorEntry.h"
#include "SensorProfiles.h"

SensorMeasurementFunc SensorEntry::getMeasurementFunc() {
  for (int i = 0; i < NUM_SENSOR_TYPES; i++) {
    if (gSensorFunctionMap[i].sensorID == sensorID) {
      return gSensorFunctionMap[i].func;
    }
  }
  return DummySensorMeasurementFunc;
}

const char* SensorEntry::getSensorID() {
  return sensorID;
}