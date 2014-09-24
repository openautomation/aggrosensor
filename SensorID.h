#pragma once

const int MAX_SENSOR_ID_LENGTH = 16;    // including null terminator
struct SensorEntry;

// every sensor type needs to define one of these and add an entry for it in gSensorEntries
typedef int (*SensorMeasurementFunc)(const struct SensorEntry *config);

// map between sensor ID (a string) and sensor reading function
struct SensorFunctionMapEntry {
  char sensorID[MAX_SENSOR_ID_LENGTH];
  SensorMeasurementFunc func;
};

// functions for looking up the function address for a sensor ID string and vice versa
SensorMeasurementFunc funcFromSensorID(const char *sensorID);
const char* idFromSensorFunc(SensorMeasurementFunc func);

// use this to not worry about calling a null function address
//int DummySensorMeasurementFunc(const struct SensorEntry *config);

