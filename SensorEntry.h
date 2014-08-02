#pragma once

////////////////////////////////////////////////////////////////////////////////////
// Globals for knowing what sensors are plugged into the arduino and on what pins
////////////////////////////////////////////////////////////////////////////////////

char gBoardName[24];
char gBoardDescription[80];

const int MAX_SENSOR_ID_LENGTH = 16;

struct SensorEntry;

// every sensor type needs to define one of these and add an entry for it in gSensorEntries
typedef int (*SensorMeasurementFunc)(const struct SensorEntry *config);

// map between sensor ID (a string) and sensor reading function
struct SensorFunctionMapEntry {
  char sensorID[MAX_SENSOR_ID_LENGTH];
  SensorMeasurementFunc func;
};

// All the info for a sensor plugged into the arduino
struct SensorEntry {
  //char sensorID[MAX_SENSOR_ID_LENGTH];  // the "class" type
  SensorMeasurementFunc func;           // the sensor "type", this will be a function in gSensorFuncMap
  char label[16];                       // label applied to data from this sensor
  char pins[4];                         // some number of pins used by the sensor
                                        // (order is dictated the sensor's read function)
  unsigned long msMeasurementPeriod;    // measurement period in milliseconds

  SensorEntry() {
    setEmpty();
  }
  
  void setEmpty() {
    //sensorID[0] = '\0';
    func = NULL;
  }
  
  bool isEmpty() const {
    return func == NULL;
  }
  
  // TODO add a separate flag for enabled!
  bool isDisabled() const {
    return msMeasurementPeriod == 0;
  }
  
  void schedule(unsigned long now) {
    //dateNextMeasurement = now + msMeasurementPeriod;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Use these 2 methods because we might change how an entry's sensor is identified
  const char* getSensorID();
  SensorMeasurementFunc getMeasurementFunc();  
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  // send data message
  void packageDataMessage(float data) const;

  // pre-allocated array for sensors
  // an entry is unused if it's sensorID is ""
  static const int NUM_SENSOR_ENTRIES = 16;
  static SensorEntry gSensorEntries[NUM_SENSOR_ENTRIES];
  
  static SensorFunctionMapEntry gSensorFunctionMap[];

  static int DummySensorMeasurementFunc(const struct SensorEntry *config) { return 0; }
};  
